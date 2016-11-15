#include "stdafx.h"
#include "Game.h"
#include "Sprite.h"
#include "GameConstants.h"


Game::Game():
    _viewportSize(GAME_FIELD_SIZE),
    _input(),
    _frameCounter(),
    _timeEaters(),

    _exitGameSignal(false)
{
    _viewport.resize(_viewportSize);
}

Game::~Game()
{}

void Game::_initStaticTimeEaters()
{
    const double now = g_now.sec();
    foreach(const weak_ptr<TimeEater>& item, _timeEaters)
    {
        if( shared_ptr<TimeEater> itemLocked = item.lock() )
            itemLocked->laterInit(now);
    }
}

void Game::_init()
{
    _statusLine.reset(new StatusLine(_viewportSize));

    _gun.reset(new Gun(_input, _viewportSize));
    _timeEaters.push_back(_gun); // TODO: how can we avoid manual doing of this?

    // all shortcuts ending the game
    _input.listenShortcut(
        Shortcut(VK_F4, Shortcut_withAlt, bind(&Game::_onExitGame, this)));
    _input.listenShortcut(
        Shortcut('X', Shortcut_withAlt, bind(&Game::_onExitGame, this)));
    _input.listenShortcut(
        Shortcut('Q', Shortcut_withAlt, bind(&Game::_onExitGame, this)));
    _input.listenShortcut(
        Shortcut('Q', Shortcut_withCtrl, bind(&Game::_onExitGame, this)));
    
    _initStaticTimeEaters();
}

void Game::run()
{
    cout << "Game started" << endl;

    _init();

    _viewport.switchDisplay(Viewport::Mode_game);

    timers::TimerEx timer;
    _input.start();
    _frameCounter = 0;
    while(!_exitGameSignal)
    {
        // Frame started
        timer.reset();

        // changes the game state with new input events
        _input.processWaitingEvents();

        // update state up to current moment
        _updateStateByTime();

        // builds and displays new frame
        _buildFrame();

        // cleaning up
        _cleanup();

        // sleep up to the end of current frame
        // negative sleep time is OK, no sleeping
        timers::SleepPrecSec(FRAME_TIME - timer.passedSec());

        // Frame ended
        ++_frameCounter;
    }

    _input.nowEnough();
    _viewport.switchDisplay(Viewport::Mode_stdout);
    cout << "Game ended" << endl;
}

void Game::_onExitGame()
{
    _exitGameSignal = true;
}

void Game::_updateStateByTime()
{
    const double now = g_now.sec();

    foreach(const weak_ptr<TimeEater>& item, _timeEaters)
    {
        if( shared_ptr<TimeEater> itemLocked = item.lock() )
            itemLocked->eatTimeUpTo(now);
    }
}

void Game::_buildFrame()
{
    _viewport.eraseDrawFrame();

    _statusLine->drawYourself(_viewport);
    _gun->drawYourself(_viewport);

    _viewport.switchFrame();
}

void Game::_cleanup()
{
    if( (_frameCounter % CLEAN_UP_EVERY_NTH_FRAME) != 0 )
        return;

    ::cleanup(_timeEaters);
}
