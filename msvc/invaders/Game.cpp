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

    _viewportSize = GameStaticObjects::minViewportSize();
    _viewportSize.x = (std::max)(_viewportSize.x, GetSystemMetrics(SM_CXMIN));
    _viewportSize.y = (std::max)(_viewportSize.y, GetSystemMetrics(SM_CYMIN));

    _objs->init(_viewportSize, _input);

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

    _viewport.gameMode(_viewportSize);

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
    _viewport.stdMode();
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

    _objs->sweetHome->drawYourself(_viewport);
    _objs->gun->drawYourself(_viewport);

    // status line better to be last
    _objs->statusLine->drawYourself(_viewport);

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

class GameFieldLayout
{
    DisplayRect _rect;

public:
    GameFieldLayout(const DisplayCoords& viewportSize):
        _rect{DisplayCoords(0, viewportSize.y), viewportSize.x, 0}
    {}

    void gap(int gapY)
    {
        _rect.br.y -= gapY;
    }

    DisplayRect nextField(int height)
    {
        DisplayRect ret = _rect;
        ret.tl.y = ret.br.y - height;
        _rect.br.y = ret.tl.y;
        return ret;
    }
};

static const int SKY_HEIGHT = 25;

DisplayCoords GameStaticObjects::minViewportSize()
{
    DisplayCoords size = SweetHome::minSize();
    size.y += Gun::height();
    size.y += StatusLine::height();
    size.y += SKY_HEIGHT;
    size.y += 1; // gaps

    return size;
}

void GameStaticObjects::init(const DisplayCoords& viewportSize, InputProcessor& input)
{
    // размещаем объекты снизу вверх
    GameFieldLayout gfl{viewportSize};

    statusLine.reset(new StatusLine{gfl.nextField(StatusLine::height())});

    sweetHome.reset(new SweetHome{gfl.nextField(SweetHome::minSize().y)});

    gfl.gap(1);

    gun.reset(new Gun{input, gfl.nextField(Gun::height())});
}
