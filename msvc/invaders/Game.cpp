#include "stdafx.h"
#include "Game.h"
#include "Sprite.h"
#include "InvaderShips.h"
#include "GameConstants.h"


Game::Game()
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
    g_random.seed(static_cast<uint32_t>(std::time(0)));

    _objs.reset(new GameStaticObjects);

    _viewportSize = GameStaticObjects::minViewportSize();
    _viewportSize.x = (std::max)(_viewportSize.x, GetSystemMetrics(SM_CXMIN));
    _viewportSize.y = (std::max)(_viewportSize.y, GetSystemMetrics(SM_CYMIN));

    _objs->init(_viewportSize, _input, _timeEaters);

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

    // _timeEaters can grow during enumeration, so we can't use iterators and
    // must not save size()
    for(std::size_t i = 0; i < _timeEaters.size(); ++i)
    {
        auto item = _timeEaters[i];
        if( shared_ptr<TimeEater> itemLocked = item.lock() )
            itemLocked->eatTimeUpTo(now);
    }
}

void Game::_buildFrame()
{
    _viewport.eraseDrawFrame();

    _objs->sweetHome->drawYourself(_viewport);

    _objs->shipManager->drawYourself(_viewport);

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
    _objs->shipManager->cleanup();
}

GameStaticObjects::GameStaticObjects()
{}

GameStaticObjects::~GameStaticObjects()
{}

DisplayCoords GameStaticObjects::minViewportSize()
{
    DisplayCoords size = SweetHome::minSize();
    size.y += Gun::height();
    size.y += StatusLine::height();
    size.y += Sky::height();
    size.y += 1; // gaps

    return size;
}

void GameStaticObjects::init(const DisplayCoords& viewportSize, InputProcessor& input,
    TimeEaters& timeEaters)
{
    DisplayAreaVertLayout layout{{DisplayCoords(0, viewportSize.y), viewportSize.x, 0}};

    statusLine.reset(new StatusLine{layout.nextField(StatusLine::height())});

    sweetHome.reset(new SweetHome{layout.nextField(SweetHome::minSize().y)});

    layout.gap(1);

    gun.reset(new Gun{input, layout.nextField(Gun::height())});
    timeEaters.push_back(gun);

    sky.reset(new Sky{layout.restArea()});

    shipManager.reset(new ShipManager{sky, timeEaters});
    timeEaters.push_back(shipManager);
}

ShipManager::ShipManager(const shared_ptr<Sky>& sky, TimeEaters& timeEaters):
    TimeEater{willBeInitedLater},
    _sky{sky},
    _timeEaters{timeEaters},
    _left{0.5}
{}

ShipManager::~ShipManager()
{}

void ShipManager::eatTime(const double from, const double to)
{
    _left -= (to - from);

    // remove all the ships flew away
    foreach(ShipPtr& ship, _ships)
    {
        if( ship && ship->flewAway() )
            ship.reset();
    }

    // may be new ships?
    if( _left < 0 )
    {
        _left = SHIP_BASIC_FREQUENCY;

        const auto echelon = _sky->randomLowEchelon();

        _ships.push_back(ShipPtr(
            new RegularBomber{to, echelon.first, echelon.second ? -1 : +1}
        ));
        _timeEaters.push_back(_ships.back());
    }
}

void ShipManager::drawYourself(Viewport& viewport)
{
    foreach(const ShipPtr& ship, _ships)
    {
        if( ship )
            ship->drawYourself(viewport);
    }
}

void ShipManager::cleanup()
{
    ::cleanup(_ships);
}