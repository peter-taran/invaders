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
    moveToRequested(),
    moveTo(),
    direction()
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
{
    DWORD mode = 0;
    GetConsoleMode(_console, &mode);

    // disable edit with mouse by default
    if( mode & ENABLE_EXTENDED_FLAGS )
    {
        mode &= ~ENABLE_QUICK_EDIT_MODE;
        SetConsoleMode(_console, mode);
    }
}

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

void InputProcessor::nowEnough()
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

void InputProcessor::collectCycle()
{
    static const DWORD MAX_READ_PORTION = 32;
    array<INPUT_RECORD, MAX_READ_PORTION> recs;

    while(_working < 2)
    {
        // waiting for input events at most FRAME_TIME, next check exit flag
        if( WAIT_OBJECT_0 ==
            WaitForSingleObject(_console, static_cast<DWORD>(FRAME_TIME * 1000)) )
        {
            // we have events
            DWORD count = 0;
            GetNumberOfConsoleInputEvents(_console, &count);
            const DWORD readCount = (std::min)(MAX_READ_PORTION, count);
            if( readCount > 0 )
            {
                boost::lock_guard<boost::mutex> singlethreaded(_queueAccess);
                DWORD readedCount = 0;
                if( ReadConsoleInput(_console, recs.data(), readCount, &readedCount) )
                {
                    assert(readedCount <= readCount); // IRL that must be equal
                    if( _working == 1 )
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

void InputProcessor::processWaitingEvents()
{
    InputEvents events;
    events.reserve(64);

    {
        boost::lock_guard<boost::mutex> singlethreaded(_queueAccess);
        events.swap(_queue);
    }

    foreach(const InputEvent& e, events)
    {
        _handleEvent(e);
    }
}

void InputProcessor::_handleEvent(const InputEvent& e)
{
    bool mouseMoved = false;
    switch(e.rec.EventType)
    {
    case KEY_EVENT:
        _handleKeyEvent(e.moment, e.rec.Event.KeyEvent);
        break;
    case MOUSE_EVENT:
        _handleMouseEvent(e.moment, e.rec.Event.MouseEvent, mouseMoved);
        break;
    default:
        return; // ignore everything else
    }

    // calculate new controller state and fire listeners when changed
    _updateState_gunMove(e.moment, mouseMoved);
    _updateState_gunFire(e.moment);
}

void InputProcessor::_handleMouseEvent(const double moment,
    const MOUSE_EVENT_RECORD& rec, bool& mouseMoved)
{
    const bool posChanged = (_inputState.mousePos != rec.dwMousePosition);
    if( posChanged )
        _inputState.mousePos = rec.dwMousePosition;

    _syncControlButtonStates(rec.dwControlKeyState);
    _syncMouseButtonStates(rec.dwButtonState);

    switch(rec.dwEventFlags)
    {
    case MOUSE_MOVED:
        mouseMoved = posChanged;
        break;
    // case MOUSE_WHEELED: maybe later
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
    ButtonState* btn = nullptr;
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

        // update button state for now and future
        const bool btnSwitched =
            _syncButtonState(shortcut.btnState, FALSE != rec.bKeyDown);

        // shortcut is a key press, not a key release
        if( !rec.bKeyDown )
            continue;
        
        // bad joke, but better to keep ourselves safe
        if( !shortcut.listener )
            continue;
        
        // repeated event about same press?
        if( shortcut.callOnce && !btnSwitched )
            continue;

        // check Alt state
        if( (0 != (shortcut.flags & Shortcut_withAlt)) != _inputState.btnAlt.pressed )
            continue;
        
        // check Ctrl state
        if( (0 != (shortcut.flags & Shortcut_withCtrl)) != _inputState.btnCtrl.pressed )
            continue;

        // this is our shortcut!
        shortcut.listener();
    }
}

void InputProcessor::_updateState_gunMove(const double moment, bool mouseMoveHappened)
{
    const InputController::MoveMode was = _state.gunMoveMode;

    if( mouseMoveHappened )
    {
        _state.gunMoveMode.moveToRequested = true;
        _state.gunMoveMode.moveTo = _inputState.mousePos.x;
    }

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
        _onChange_gunMove(Command<InputController::MoveMode>(moment, _state.gunMoveMode));
    }
}

void InputProcessor::_updateState_gunFire(const double moment)
{
    const InputController::FireMode was = _state.gunFireMode;

    _state.gunFireMode.opened =
        _inputState.btnSpace.pressed ||
        _inputState.btnMouse1.pressed ||
        _inputState.btnMouse2.pressed;

    if( was != _state.gunFireMode && _onChange_gunFire )
    {
        _onChange_gunFire(Command<InputController::FireMode>(moment, _state.gunFireMode));
    }
}
