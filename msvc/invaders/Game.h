#pragma once

#include "Viewport.h"
#include "Input.h"
#include "Gun.h"
#include "StatusLine.h"
#include "SweetHome.h"
#include "Sky.h"
#include "InvaderShips.h"


typedef std::deque< weak_ptr<TimeEater> > TimeEaters;

class ShipManager: public TimeEater, public Drawable
{
public:
    ShipManager(const shared_ptr<Sky>& sky, TimeEaters& timeEaters);
    ~ShipManager();

    virtual void eatTime(const double from, const double to);
    virtual void drawYourself(Viewport& viewport);

    void cleanup();

private:
    shared_ptr<Sky> _sky;
    TimeEaters& _timeEaters;

    double _left;

    typedef shared_ptr<InvaderShip> ShipPtr;
    deque<ShipPtr> _ships;
};

// Responsible for static (always existing) game objects and their layout
struct GameStaticObjects: noncopyable
{
    GameStaticObjects(); // do not create objects
    ~GameStaticObjects();

    // what minimal viewport size can accomodate all the objects
    static DisplayCoords minViewportSize();

    // creates all objects
    void init(const DisplayCoords& viewportSize, InputProcessor& input,
        TimeEaters& timeEaters);

    // objects
    shared_ptr<StatusLine> statusLine;
    shared_ptr<Gun> gun;
    shared_ptr<SweetHome> sweetHome;
    shared_ptr<Sky> sky;
    shared_ptr<ShipManager> shipManager;
};

class Game: noncopyable
{
public:
    Game();
    ~Game();

    void run();

private:
    Viewport _viewport;
    DisplayCoords _viewportSize;
    
    InputProcessor _input;

    long long _frameCounter;
    
    TimeEaters _timeEaters;

    // static game objects
    scoped_ptr<GameStaticObjects> _objs;

    void _init();
    void _initStaticTimeEaters();

    void _updateStateByTime();
    void _buildFrame();
    void _cleanup();

    bool _exitGameSignal = false;
    void _onExitGame();
};

