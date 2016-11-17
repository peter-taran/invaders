#include "stdafx.h"
#include "Game.h"
#include "Sprite.h"
#include "GameConstants.h"


Game::Game():
    _input(),
    _frameCounter(),
    _timeEaters(),

    _exitGameSignal(false)
{}

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
    _objs.reset(new GameStaticObjects);

    DisplayCoords viewportSize = GameStaticObjects::minViewportSize();
    viewportSize.x = (std::max)(viewportSize.x, GetSystemMetrics(SM_CXMIN));
    viewportSize.y = (std::max)(viewportSize.y, GetSystemMetrics(SM_CYMIN));
    
    _viewport.resize(viewportSize);

    _objs->init(viewportSize, _input);

    _timeEaters.push_back(_objs->gun); // TODO how can we avoid manual doing of this?

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

    _objs->statusLine->drawYourself(_viewport);
    _objs->sweetHome->drawYourself(_viewport);
    _objs->gun->drawYourself(_viewport);

    _viewport.switchFrame();
}

void Game::_cleanup()
{
    if( (_frameCounter % CLEAN_UP_EVERY_NTH_FRAME) != 0 )
        return;

    ::cleanup(_timeEaters);
}

GameStaticObjects::GameStaticObjects()
{}

GameStaticObjects::~GameStaticObjects()
{}

static const int SKY_HEIGHT = 20;

DisplayCoords GameStaticObjects::minViewportSize()
{
    DisplayCoords size = SweetHome::minSize();
    size.y += Gun::height();
    size.y += StatusLine::height();
    size.y += SKY_HEIGHT;
    size.y += 2; // spaces

    return size;
}

void GameStaticObjects::init(const DisplayCoords& viewportSize, InputProcessor& input)
{
    int y = 0;
    int height = 0;
    
    y += SKY_HEIGHT; // skip sky yet

    height = Gun::height();
    gun.reset(new Gun(input, {{0, y}, viewportSize.x, height}));
    y += height;

    ++y; // space between

    height = SweetHome::minSize().y;
    sweetHome.reset(new SweetHome({{0, y}, viewportSize.x, height}));
    y += height;

    ++y; // space between

    height = StatusLine::height();
    statusLine.reset(new StatusLine({{0, y}, viewportSize.x, height}));
    y += height;
}
