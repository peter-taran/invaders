#pragma once

#include "Viewport.h"
#include "Input.h"
#include "Gun.h"
#include "StatusLine.h"


class Game: noncopyable
{
public:
    Game();
    ~Game();

    void run();

private:
    const DisplayCoord _viewportSize;
    Viewport _viewport;
    
    InputProcessor _input;

    long long _frameCounter;
    
    typedef std::deque< weak_ptr<TimeEater> > TimeEaters;
    TimeEaters _timeEaters;

    // static game objects
    shared_ptr<Gun> _gun;
    shared_ptr<StatusLine> _statusLine;

    void _init();
    void _initStaticTimeEaters();

    void _updateStateByTime();
    void _buildFrame();
    void _cleanup();

    bool _exitGameSignal;
    void _onExitGame();
};

