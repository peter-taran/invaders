#pragma once

#include "Viewport.h"
#include "InputController.h"
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
    
    InputController _input;

    StatusLine _statusLine;
    Gun _spaag; // gun

    bool _changeState();
    void _buildFrame();
};

