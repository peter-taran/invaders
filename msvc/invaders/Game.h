#pragma once

#include "Viewport.h"
#include "Input.h"
#include "Gun.h"
#include "StatusLine.h"
#include "SweetHome.h"
#include "Sky.h"


// Responsible for static (always existing) game objects and their layout
struct GameStaticObjects: public Drawable, noncopyable
{
    GameStaticObjects(const DisplayCoords& viewportSize, InputProcessor& input,
        struct GameControllers& controllers);
    ~GameStaticObjects();

    // what minimal viewport size can accomodate all the objects
    static DisplayCoords minViewportSize();

    // objects
    shared_ptr<StatusLine> statusLine;
    shared_ptr<Gun> gun;
    shared_ptr<SweetHome> sweetHome;
    shared_ptr<Sky> sky;

    // interfaces implementation
    virtual void drawYourself(class Viewport& viewport);
};

class Cleaner: public Cleanee
{
public:
    Cleaner(GameControllers& controllers);
    ~Cleaner();

    void put(const shared_ptr<Cleanee>& object);

    virtual void cleanUp();

private:
    GameControllers& _controllers;
    deque<weak_ptr<Cleanee>> _objects;
};

class TimeEaters: public Cleanee, noncopyable
{
public:
    TimeEaters();
    ~TimeEaters();

    void put(const shared_ptr<TimeEater>& object);
    void initAll(const double moment);
    void eatTime(const double now);
    virtual void cleanUp();

private:
    deque<weak_ptr<TimeEater>> _eaters;
};

class Screenplay: public noncopyable
{
public:
    Screenplay(GameControllers& controllers);
    ~Screenplay();

    void start(const shared_ptr<Sky>& sky, const double moment);

    void play(const double moment);

private:
    GameControllers& _controllers;
    shared_ptr<Sky> _sky;

    double _lastShipBirth {};
};

class TransientsManager: public Drawable, public Cleanee, noncopyable
{
public:
    TransientsManager();
    ~TransientsManager();

    void put(const shared_ptr<TransientDrawable>& object);
    void killThoseWhoseTimeHasCome();
    virtual void drawYourself(Viewport& viewport);
    virtual void cleanUp();

private:
    deque<shared_ptr<TransientDrawable>> _objects;
};

// Root game controllers: garbage collector, list of time eaters, etc
struct GameControllers: noncopyable
{
    // number of current frame, starting with 0
    long long frameCounter = 0;

    // manages object who need to be cleaned sometimes
    Cleaner cleaner;

    // all the time eaters
    TimeEaters timeEaters;

    // all transient objects
    TransientsManager transients;

    // screenplay object
    Screenplay screenplay;

    GameControllers();
    ~GameControllers();
};

class Game: noncopyable
{
public:
    Game();
    ~Game();

    void run();

private:
    Viewport _viewport; // must be first, to initialize console window properly
    DisplayCoords _viewportSize;
    InputProcessor _input;

    scoped_ptr<GameControllers> _controllers;
    scoped_ptr<GameStaticObjects> _objs;

    void _init(const double moment);

    void _buildFrame();

    bool _exitGameSignal = false;
    void _onExitGame();
};

