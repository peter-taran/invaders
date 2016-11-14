#include "stdafx.h"
#include "Input.h"
#include "GameConstants.h"


Shortcut::Shortcut(const WORD virtualKeyCode, const WORD flags,
    const ShortcutListener& listener, bool callOnce)
:
    virtualKeyCode(virtualKeyCode),
    flags(flags),
    callOnce(callOnce),
    listener(listener)
{}

InputController::FireMode::FireMode():
    opened(false)
{}

bool InputController::FireMode::operator==(const FireMode& cmpWith) const
{
    return opened == cmpWith.opened;
}

InputController::MoveMode::MoveMode():
    direction(0)
{}

bool InputController::MoveMode::operator==(const MoveMode& cmpWith) const
{
    return
        moveToRequested == cmpWith.moveToRequested &&
        moveTo          == cmpWith.moveTo &&
        direction       == cmpWith.direction;
}

InputController::InputController()
{}

InputProcessor::InputProcessor():
    _console(GetStdHandle(STD_INPUT_HANDLE)),

    // last ones
    _working(0),
    _collectThread(bind(&InputProcessor::collectCycle, this))
{}

InputProcessor::~InputProcessor()
{}

InputProcessor::InputEvent::InputEvent():
    moment(0)
{
    zeroVar(rec);
}

InputProcessor::InputEvent::InputEvent(const INPUT_RECORD& rec):
    moment(g_now.sec()),
    rec(rec)
{}

InputProcessor::ButtonState::ButtonState():
    pressed(false)
{}

bool InputProcessor::ButtonState::operator==(const ButtonState& cmpWith) const
{
    return pressed == cmpWith.pressed;
}

InputProcessor::InputState::InputState()
{}

void InputProcessor::start()
{
    InterlockedExchange(&_working, 1);
}

void InputProcessor::nowEnought()
{
    InterlockedExchange(&_working, 2);
    _collectThread.join();
}

InputProcessor::ShortcutEx::ShortcutEx(const Shortcut& from):
    Shortcut(from),
    btnState()
{}

void InputProcessor::listenShortcut(const Shortcut& shortcut)
{
    _shortcuts.push_back(shortcut);
}

void InputProcessor::listenGunFireModeChange(FireModeListener listener)
{
    _onChange_gunFire = listener;
}

void InputProcessor::listenGunMoveModeChange(MoveModeListener listener)
{
    _onChange_gunMove = listener;
}

void InputProcessor::processWaitingEvents()
{
    InputEvents events;
    events.reserve(32);

    {
        boost::lock_guard<boost::mutex> singlethreaded(_queueAccess);
        events.swap(_queue);
    }

    foreach(const InputEvent& e, events)
    {
        _handleEvent(e);
    }
}

void InputProcessor::collectCycle()
{
    timers::TicksCvt tcvt;

    while(_working < 2)
    {
        static const DWORD MAX_READ_PORTION = 32;
        array<INPUT_RECORD, MAX_READ_PORTION> recs;

        // waiting for input events at most FRAME_TIME, next checking exit flag
        if( WAIT_OBJECT_0 ==
            WaitForSingleObject(_console, static_cast<DWORD>(FRAME_TIME * 1000)) )
        {
            // we have event, collect it
            DWORD count = 0;
            GetNumberOfConsoleInputEvents(_console, &count);
            const DWORD readCount = (std::min)(MAX_READ_PORTION, count);
            if( readCount > 0 )
            {
                boost::lock_guard<boost::mutex> singlethreaded(_queueAccess);
                DWORD readedCount = 0;
                if( ReadConsoleInput(_console, recs.data(), readCount, &readedCount) )
                {
                    assert(readedCount <= readCount); // really that must be equal
                    if( _working > 0 )
                    {
                        const INPUT_RECORD* const itrEnd = recs.data() + readedCount;
                        for(const INPUT_RECORD* itr = recs.data(); itr != itrEnd; ++itr)
                            _queue.push_back(*itr);
                    }
                }
            }
        }
    }
}

void InputProcessor::_handleKeyEvent(const double moment, const KEY_EVENT_RECORD& rec)
{
    _syncControlButtonStates(rec.dwControlKeyState);
    _syncFiredButtonState(rec);

    _handleShortcut(moment, rec);
}

void InputProcessor::_syncControlButtonStates(DWORD controlKeysState)
{
    _syncButtonState(_inputState.btnAlt,
        0 != (controlKeysState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)));
    
    _syncButtonState(_inputState.btnCtrl,
        0 != (controlKeysState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)));
}

void InputProcessor::_syncMouseButtonStates(DWORD mouseButtonsState)
{
   _syncButtonState(_inputState.btnMouse1,
        0 != (mouseButtonsState & FROM_LEFT_1ST_BUTTON_PRESSED));
   _syncButtonState(_inputState.btnMouse2,
        0 != (mouseButtonsState & RIGHTMOST_BUTTON_PRESSED));
 }

bool InputProcessor::_syncFiredButtonState(const KEY_EVENT_RECORD& rec)
{
    ButtonState* btn = NULL;
    switch(rec.wVirtualKeyCode)
    {
    case VK_LEFT:  btn = &_inputState.btnLeft; break;
    case VK_RIGHT: btn = &_inputState.btnRight; break;
    case VK_SPACE: btn = &_inputState.btnSpace; break;

    default: return false;
    }
    
    assert(btn);
    return _syncButtonState(*btn, FALSE != rec.bKeyDown);
}

bool InputProcessor::_syncButtonState(ButtonState& btn, bool pressedNow)
{
    if( pressedNow )
    {
        // button down
        if( !btn.pressed )
        {
            btn.pressed = true;
            return true;
        }
    }
    else
    {
        // button up
        if( btn.pressed )
        {
            btn.pressed = false;
            return true;
        }
    }
    return false;
}

void InputProcessor::_handleShortcut(const double moment, const KEY_EVENT_RECORD& rec)
{

    foreach(ShortcutEx& shortcut, _shortcuts)
    {
        // check button
        if( rec.wVirtualKeyCode != shortcut.virtualKeyCode )
            continue;

        // update button state for future
        const bool btnSwitched =
            _syncButtonState(shortcut.btnState, FALSE != rec.bKeyDown);

        // shortcut is a key press
        if( !rec.bKeyDown )
            continue;
        
        // bad joke, but better to keep ourselves safe
        if( !shortcut.listener )
            continue;
        
        // repeat button event?
        if( shortcut.callOnce && !btnSwitched )
            continue;

        // check alt state
        if( (0 != (shortcut.flags & Shortcut_withAlt)) != _inputState.btnAlt.pressed )
            continue;
        
        // check control state
        if( (0 != (shortcut.flags & Shortcut_withCtrl)) != _inputState.btnCtrl.pressed )
            continue;

        // this is our shortcut!
        shortcut.listener();
    }
}

void InputProcessor::_handleMouseEvent(const double moment,
    const MOUSE_EVENT_RECORD& rec)
{
    _inputState.mousePos = rec.dwMousePosition;

    _syncControlButtonStates(rec.dwControlKeyState);
    _syncMouseButtonStates(rec.dwButtonState);

    // if we will need mouse wheel, process
    // rec.dwEventFlags in (MOUSE_HWHEELED, MOUSE_WHEELED)
}

void InputProcessor::_handleEvent(const InputEvent& e)
{
    switch(e.rec.EventType)
    {
    case KEY_EVENT:
        _handleKeyEvent(e.moment, e.rec.Event.KeyEvent);
        break;
    case MOUSE_EVENT:
        _handleMouseEvent(e.moment, e.rec.Event.MouseEvent);
        break;
    default:
        return; // ignore everything else
    }

    // calculate new controller state and fire listeners when changed
    _updateState_gunMove(e.moment);
    _updateState_gunFire(e.moment);
}

void InputProcessor::_updateState_gunMove(const double moment)
{
    // TODO: here is problem, we should turn on "moveTo" mode
    // only when mouse moved, but we have no this info

    InputController::MoveMode was = _state.gunMoveMode;
    if( _inputState.btnLeft.pressed == _inputState.btnRight.pressed )
    {
        _state.gunMoveMode.direction = 0;
    }
    else if( _inputState.btnLeft.pressed )
    {
        _state.gunMoveMode.direction = -1;
    }
    else
    {
        _state.gunMoveMode.direction = +1;
    }

    if( was != _state.gunMoveMode && _onChange_gunMove )
    {
        _onChange_gunMove(moment, was, _state.gunMoveMode);
    }
}

void InputProcessor::_updateState_gunFire(const double moment)
{
    InputController::FireMode was = _state.gunFireMode;

    _state.gunFireMode.opened =
        _inputState.btnSpace.pressed ||
        _inputState.btnMouse1.pressed ||
        _inputState.btnMouse2.pressed;

    if( was != _state.gunFireMode && _onChange_gunFire )
    {
        _onChange_gunFire(moment, was, _state.gunFireMode);
    }
}
