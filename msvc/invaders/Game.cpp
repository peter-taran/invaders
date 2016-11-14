#include "stdafx.h"
#include "Game.h"
#include "Sprite.h"
#include "GameConstants.h"


Game::Game():
    _viewportSize(GAME_FIELD_SIZE),
    _input(),
    _statusLine(_viewportSize),
    _gun(_input, _viewportSize),

    _exitGameSignal(false)
{
    _viewport.resize(_viewportSize);
}

Game::~Game()
{}

void Game::_init()
{
    // all shortcuts ending the game
    _input.listenShortcut(
        Shortcut(VK_F4, Shortcut_withAlt, bind(&Game::_onExitGame, this)));
    _input.listenShortcut(
        Shortcut('X', Shortcut_withAlt, bind(&Game::_onExitGame, this)));
    _input.listenShortcut(
        Shortcut('Q', Shortcut_withAlt, bind(&Game::_onExitGame, this)));
    _input.listenShortcut(
        Shortcut('Q', Shortcut_withCtrl, bind(&Game::_onExitGame, this)));
}

void Game::run()
{
    _init();

    cout << "Game started" << endl;
    _viewport.switchDisplay(Viewport::Mode_game);
    _input.start();

    // Frame cycle
    timers::TimerEx timer;
    while(!_exitGameSignal)
    {
        timer.reset();

        // changes the game state with new input events
        _input.processWaitingEvents();

        // TODO: desing time eating object to two calls:
        //   a) update state up to event happened time
        //   b) process event without moment argument
        // can collisions be adopted to this?

        // update state up to current moment
        _updateStateByTime();

        // builds and displays new frame
        _buildFrame();

        // sleep up to the end of current frame
        // negative sleep time is OK, no sleeping
        timers::SleepPrecSec(FRAME_TIME - timer.passedSec());
    }

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

    _statusLine.updateStateByTime(now);
    _gun.updateStateByTime(now);
}

void Game::_buildFrame()
{
    _viewport.eraseDrawFrame();

    _statusLine.drawYourself(_viewport);
    _gun.drawYourself(_viewport);

    _viewport.switchFrame();
}
