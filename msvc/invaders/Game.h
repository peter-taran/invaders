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

    StatusLine _statusLine;
    Gun _gun; // gun

    void _init();
    void _buildFrame();
    void _updateStateByTime();

    bool _exitGameSignal;
    void _onExitGame();
};

