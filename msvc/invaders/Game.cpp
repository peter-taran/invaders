#include "stdafx.h"
#include "Game.h"
#include "Sprite.h"
#include "InvaderShips.h"
#include "GameConstants.h"


Game::Game()
{}

Game::~Game()
{}

void Game::run()
{
    cout << "Game started" << endl;

    timers::TimerEx timer;
    double frameMoment = g_now.sec();

    _init(frameMoment);

    _viewport.gameMode(_viewportSize);

    _input.start();
    while(!_exitGameSignal)
    {
        // Frame started
        timer.reset();
        frameMoment = g_now.sec();

        // changes the game state with new input events
        _input.processWaitingEvents();

        // updates state up to current moment
        _controllers->timeEaters.eatTime(frameMoment);

        // generate screenplay events
        _controllers->screenplay.play(frameMoment);

        // kill out of game objects
        _controllers->transients.killThoseWhoseTimeHasCome();

        // collisions processing
        // TODO

        // cleaning garbage sometimes
        _controllers->cleaner.cleanUp();

        // builds and displays new frame
        _buildFrame();

        // sleep up to the end of current frame
        // negative sleep time is OK, no sleeping
        timers::SleepPrecSec(FRAME_TIME - timer.passedSec());

        // Frame ended
        ++_controllers->frameCounter;
    }

    _input.nowEnough();
    _viewport.stdMode();

    cout << "Game ended" << endl;
}

void Game::_init(const double moment)
{
    g_random.seed(static_cast<uint32_t>(std::time(0)));

    _controllers.reset(new GameControllers);

    _viewportSize = GameStaticObjects::minViewportSize();
    _viewportSize.x = (std::max)(_viewportSize.x, GetSystemMetrics(SM_CXMIN));
    _viewportSize.y = (std::max)(_viewportSize.y, GetSystemMetrics(SM_CYMIN));

    _objs.reset(new GameStaticObjects(_viewportSize, _input, *_controllers));

    // all shortcuts ending the game
    _input.listenShortcut(
        Shortcut(VK_F4, Shortcut_withAlt, bind(&Game::_onExitGame, this)));
    _input.listenShortcut(
        Shortcut('X', Shortcut_withAlt, bind(&Game::_onExitGame, this)));
    _input.listenShortcut(
        Shortcut('Q', Shortcut_withAlt, bind(&Game::_onExitGame, this)));
    _input.listenShortcut(
        Shortcut('Q', Shortcut_withCtrl, bind(&Game::_onExitGame, this)));

    // initing static time eaters
    _controllers->timeEaters.initAll(moment);

    // starting screenplay
    _controllers->screenplay.start(_objs->sky, moment);
}

void Game::_onExitGame()
{
    _exitGameSignal = true;
}

void Game::_buildFrame()
{
    _viewport.eraseDrawFrame();

    _controllers->transients.drawYourself(_viewport);
    _objs->drawYourself(_viewport);

    _viewport.switchFrame();
}

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

GameStaticObjects::GameStaticObjects(const DisplayCoords& viewportSize,
    InputProcessor& input, GameControllers& controllers)
{
    DisplayAreaVertLayout layout{{DisplayCoords(0, viewportSize.y), viewportSize.x, 0}};

    statusLine.reset(new StatusLine{layout.nextField(StatusLine::height())});

    sweetHome.reset(new SweetHome{layout.nextField(SweetHome::minSize().y)});

    layout.gap(1);

    gun.reset(new Gun{layout.nextField(Gun::height())});
    controllers.timeEaters.put(gun);
    input.listenGunMoveModeChange(bind(&Gun::commandMove, gun.get(), _1));
    input.listenGunFireModeChange(bind(&Gun::commandFire, gun.get(), _1));

    sky.reset(new Sky{layout.restArea()});
}

void GameStaticObjects::drawYourself(class Viewport& viewport)
{
    sweetHome->drawYourself(viewport);

    gun->drawYourself(viewport);

    // status line better to be last
    statusLine->drawYourself(viewport);
}

TransientsManager::TransientsManager()
{}

TransientsManager::~TransientsManager()
{}

void TransientsManager::put(const shared_ptr<TransientDrawable>& object)
{
    _objects.push_back(object);
}

void TransientsManager::killThoseWhoseTimeHasCome()
{
    foreach(auto& object, _objects)
    {
        if( object && object->timeToDie() )
        {
            object.reset();
        }
    }
}

void TransientsManager::drawYourself(Viewport& viewport)
{
    foreach(const auto& object, _objects)
    {
        if( object )
        {
            object->drawYourself(viewport);
        }
    }
}

void TransientsManager::cleanUp()
{
    ::cleanup(_objects);
}

Screenplay::Screenplay(GameControllers& controllers):
    _controllers{controllers}
{}

Screenplay::~Screenplay()
{}

void Screenplay::start(const shared_ptr<Sky>& sky, const double moment)
{
    _sky = sky;
    _lastShipBirth = moment - 1;
}

void Screenplay::play(const double moment)
{
    // may be new ships?
    if( (moment - _lastShipBirth) >= SHIP_BASIC_FREQUENCY )
    {
        _lastShipBirth = moment;

        const auto echelon = _sky->randomLowEchelon();

        shared_ptr<RegularBomber> ship {
            new RegularBomber{moment, echelon.first, echelon.second ? -1 : +1}
        };

        _controllers.timeEaters.put(ship);
        _controllers.transients.put(ship);
    }
}

Cleaner::Cleaner(GameControllers& controllers):
    _controllers(controllers)
{}

Cleaner::~Cleaner()
{}

void Cleaner::put(const shared_ptr<Cleanee>& object)
{
    _objects.push_back(object);
}

void Cleaner::cleanUp()
{
    if( (_controllers.frameCounter % CLEAN_UP_EVERY_NTH_FRAME) != 0 )
        return;

    // cleaning static objects
    _controllers.timeEaters.cleanUp();
    _controllers.transients.cleanUp();

    // cleaning dynamic objects
    foreach(auto& object, _objects)
    {
        if( shared_ptr<Cleanee> objectLocked = object.lock() )
        {
            objectLocked->cleanUp();
        }
    }

    ::cleanup(_objects);
}

GameControllers::GameControllers():
    cleaner(*this),
    screenplay(*this)
{}

GameControllers::~GameControllers()
{}

TimeEaters::TimeEaters()
{}

TimeEaters::~TimeEaters()
{}

void TimeEaters::put(const shared_ptr<TimeEater>& object)
{
    _eaters.push_back(object);
}

void TimeEaters::initAll(const double moment)
{
    foreach(const auto& item, _eaters)
    {
        if( shared_ptr<TimeEater> itemLocked = item.lock() )
            itemLocked->laterInit(moment);
    }
}

void TimeEaters::eatTime(const double now)
{
    foreach(const auto& eater, _eaters)
    {
        if( shared_ptr<TimeEater> eaterLocked = eater.lock() )
            eaterLocked->eatTimeUpTo(now);
    }
}

void TimeEaters::cleanUp()
{
    ::cleanup(_eaters);
}
