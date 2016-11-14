#include "stdafx.h"
#include "InputController.h"
#include "GameConstants.h"
#include <boost/bind.hpp>


static timers::Now g_now;

InputController::InputController():
    _console(GetStdHandle(STD_INPUT_HANDLE)),

    // button handlers
    _btnLeft(&InputController::onBtnLeft),
    _btnRight(&InputController::onBtnRight),
    _btnSpace(&InputController::onBtnSpace),
    
    // last ones
    _working(0),
    _collectThread(boost::bind(&InputController::collectCycle, this))
{}

InputController::~InputController()
{}

void InputController::start()
{
    InterlockedExchange(&_working, 1);
}

void InputController::nowEnought()
{
    InterlockedExchange(&_working, 2);
    _collectThread.join();
}

void InputController::collectCycle()
{
    timers::TicksCvt tcvt;

    while(_working < 2)
    {
        // waiting for input events at most FRAME_TIME, next checking exit flag
        if( WAIT_OBJECT_0 ==
            WaitForSingleObject(_console, static_cast<DWORD>(FRAME_TIME * 1000)) )
        {
            // we have event, collect it
            DWORD count = 0;
            GetNumberOfConsoleInputEvents(_console, &count);
            for(DWORD index = 0; index < count; ++index)
            {
                INPUT_RECORD rec;
                DWORD readCount = 0;
                if( ReadConsoleInput(_console, &rec, 1, &readCount) )
                {
                    assert(readCount == 1); // ReadConsoleInput return at least one event
                    if( _working > 0 )
                        _handleEvent(rec);
                }
            }
        }
    }
}

void InputController::_handleEvent(const INPUT_RECORD& rec)
{
    const double now = g_now.sec();

    boost::lock_guard<boost::mutex> singlethreaded(_queueAccess);

    switch(rec.EventType)
    {
    case KEY_EVENT:
        _handleKeyEvent(now, rec.Event.KeyEvent);
        break;
    case MOUSE_EVENT:
        _handleMouseEvent(now, rec.Event.MouseEvent);
        break;
    default:
        // ignore everything else
        break;
    }
}

void InputController::_handleKeyEvent(const double moment, const KEY_EVENT_RECORD& rec)
{
    if( _handleShortcut(moment, rec) )
        return;
    
    if( Button* btn = _syncButtonState(moment, rec) )
    {
        if( btn->onChangeState )
            btn->onChangeState(this, moment);
    }
 }

InputController::Button* InputController::_syncButtonState(const double moment,
    const KEY_EVENT_RECORD& rec)
{
    Button* btn = NULL;
    switch(rec.wVirtualKeyCode)
    {
    case VK_LEFT:  btn = &_btnLeft; break;
    case VK_RIGHT: btn = &_btnRight; break;
    case VK_SPACE: btn = &_btnSpace; break;
    default: return NULL;
    }
    
    assert(btn != NULL);

    if( rec.bKeyDown )
    {
        // button down
        if( !btn->pressedNow )
        {
            btn->pressedNow = true;
            return btn;
        }
    }
    else
    {
        // button up
        if( btn->pressedNow )
        {
            btn->pressedNow = false;
            return btn;
        }
    }

    return NULL;
}

bool InputController::_handleShortcut(const double moment, const KEY_EVENT_RECORD& rec)
{
    if( rec.bKeyDown && rec.wVirtualKeyCode == VK_F4 &&
        0 != (rec.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) )
    {
        _queue.push_back(Command(moment, Command_exitGame));
        return true;
    }

    return false;
}

void InputController::_handleMouseEvent(const double moment,
    const MOUSE_EVENT_RECORD& rec)
{
    switch(rec.dwEventFlags)
    {
    case 0: // button clicked
        // TODO: fire
        break;
    case MOUSE_MOVED:
        // TODO: move of Gun
        break;
    }
}

void InputController::onMoveBtn(const double moment, Button& btnFired,
    Button& btnOpposite, CommandType cmd, CommandType cmdOpposite)
{
    if( btnFired.pressedNow )
    {
        if( btnOpposite.pressedNow )
        {
            _queue.push_back(Command(moment, Command_moveStop));
        }
        else
        {
            _queue.push_back(Command(moment, cmd));
        }
    }
    else
    {
        if( btnOpposite.pressedNow )
        {
            _queue.push_back(Command(moment, cmdOpposite));
        }
        else
        {
            _queue.push_back(Command(moment, Command_moveStop));
        }
    }
}

void InputController::onBtnLeft(const double moment)
{
    onMoveBtn(moment, _btnLeft, _btnRight, Command_moveLeft, Command_moveRight);
}

void InputController::onBtnRight(const double moment)
{
    onMoveBtn(moment, _btnRight, _btnLeft, Command_moveRight, Command_moveLeft);
}

void InputController::onBtnSpace(const double moment)
{
    _queue.push_back(Command(
        moment,
        _btnSpace.pressedNow ? Command_fireCease : Command_fireOpen
    ));
}

Commands InputController::popCommands()
{
    boost::lock_guard<boost::mutex> singlethreaded(_queueAccess);

    Commands result;
    result.reserve(_queue.size());
    std::copy(_queue.begin(), _queue.end(), std::back_inserter(result));
    _queue.clear();
    return result;
}
