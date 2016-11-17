#pragma once

#include "Viewport.h"
#include "Input.h"
#include "Gun.h"
#include "StatusLine.h"
#include "SweetHome.h"


// Responsible for static (always existing) game objects and their layout
struct GameStaticObjects: noncopyable
{
    GameStaticObjects(); // do not create objects
    ~GameStaticObjects();

    // what minimal viewport size can accomodate all the objects
    static DisplayCoords minViewportSize();

    // creates all objects
    void init(const DisplayCoords& viewportSize, InputProcessor& input);

    // objects
    shared_ptr<Gun> gun;
    shared_ptr<StatusLine> statusLine;
    shared_ptr<SweetHome> sweetHome;
};

class Game: noncopyable
{
public:
    Game();
    ~Game();

    void run();

private:
    Viewport _viewport;
    
    InputProcessor _input;

    long long _frameCounter;
    
    typedef std::deque< weak_ptr<TimeEater> > TimeEaters;
    TimeEaters _timeEaters;

    // static game objects
    scoped_ptr<GameStaticObjects> _objs;

    void _init();
    void _initStaticTimeEaters();

    void _updateStateByTime();
    void _buildFrame();
    void _cleanup();

    bool _exitGameSignal;
    void _onExitGame();
};

