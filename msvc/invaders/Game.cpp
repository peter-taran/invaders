#include "stdafx.h"
#include "Game.h"
#include "Sprite.h"
#include "GameConstants.h"


Game::Game():
    _viewportSize(GAME_FIELD_SIZE),
    _input(),
    _statusLine(_viewportSize),
    _spaag(_viewportSize)
{
    _viewport.resize(_viewportSize);
}

Game::~Game()
{}

void Game::run()
{
    cout << "Game started" << endl;
    _viewport.switchDisplay(Viewport::Mode_game);

    // TODO: for debug
    //_viewport.draw(DisplayCoord(50, 10), Sprite(CROSS, DisplayCoord(), CharAttr(DisplayColor_lightRed, DisplayColor_black)));

    _input.start();

    // Frame cycle
    for(;;)
    {
        timers::TimerEx timer;

        // changes the game state with new input
        if( _changeState() )
            break;

        // builds and displays new frame
        _buildFrame();

        // sleep up to the end of current frame
        // negative sleep time is OK, no sleeping
        timers::SleepPrecSec(FRAME_TIME - timer.passedSec());
    }

    _viewport.switchDisplay(Viewport::Mode_stdout);
    cout << "Game ended" << endl;
}

bool Game::_changeState()
{
    Commands commands = _input.popCommands();

    foreach(const Command& cmd, commands)
    {
        if( cmd.command == Command_exitGame )
            return true;
        
        _statusLine.processCommand(cmd);
        _spaag.processCommand(cmd);
    }

    const Command commandAtEndOfFrame(timers::Now().sec(), Command_none);
    _spaag.processCommand(commandAtEndOfFrame);

    return false;
}

void Game::_buildFrame()
{
    _viewport.eraseDrawFrame();

    _statusLine.drawYourself(_viewport);
    _spaag.drawYourself(_viewport);

    _viewport.switchFrame();
}
