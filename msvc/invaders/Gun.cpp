#include "stdafx.h"
#include "Gun.h"
#include "Viewport.h"
#include "InputController.h"
#include "GameConstants.h"


/*static const array<wstring, 3> IMAGE = {
    L"    ||    ",
    L"  ((oo))  ",
    L"[==0000==]",
};*/
static const array<wstring, 3> IMAGE = {
    L"    !!    ",
    L"  ((°°))  ",
    L"[==••••==]",
};

Gun::Gun(const DisplayCoord& viewportSize):
    _positionY(0),
    _xrange(),

    _positionX(0),
    _speed(0),
    _movement(Command_none),
    _prevMoment(0),

    _image(IMAGE)
{
    const double gunWidth = _image.size().x;

    _xrange.first = (gunWidth / 2);
    _xrange.second = viewportSize.x - (gunWidth / 2) - 2;
    _positionX = (_xrange.first + _xrange.second) / 2;

    _positionY = 40; // TODO: change to appropriate
}

Gun::~Gun()
{}

void Gun::drawYourself(Viewport& viewport)
{
    __super::drawYourself(viewport);
    viewport.draw(DisplayCoord(_positionX, _positionY), _image);
}

void Gun::_doMoving(const double now)
{
    if( _movement == Command_none )
        return;
    
    // uniform component of motion
    _positionX +=
        (_movement == Command_moveLeft ? -_speed : +_speed)
        * (now - _prevMoment);

    // accelerated component of motion
    if( _speed < SPAAG_MAX_SPEED )
    {
        const double speedUpRestPerc = (SPAAG_MAX_SPEED - _speed) / SPAAG_MAX_SPEED;
        const double speedUpMoment = _prevMoment + (SPAAG_SPEEDUP_TIME * speedUpRestPerc);
        const double speedUpPeriod = (std::min)(speedUpMoment, now) - _prevMoment;

        double speedAdd =
            (SPAAG_MAX_SPEED / SPAAG_SPEEDUP_TIME) // acceleration
            * speedUpPeriod;
        _speed += speedAdd;

        if( _movement == Command_moveLeft )
            speedAdd = -speedAdd;
        _positionX += // a * t^2 / 2
            speedAdd * speedUpPeriod / 2;
    }

    _prevMoment = now;

    // bounds
    if( _positionX < _xrange.first )
    {
        _positionX = _xrange.first;
        _speed = 0;
        _movement = Command_none;
    }
    else if( _positionX > _xrange.second )
    {
        _positionX = _xrange.second;
        _speed = 0;
        _movement = Command_none;
    }
}

void Gun::processCommand(const Command& cmd)
{
    __super::processCommand(cmd);

    _doMoving(cmd.moment);

    switch(cmd.command)
    {
    case Command_moveStop:
        _speed = 0;
        _movement = Command_none;
        break;
    case Command_moveLeft:
        _speed = 0;
        _movement = Command_moveLeft;
        _prevMoment = cmd.moment;
        break;
    case Command_moveRight:
        _speed = 0;
        _movement = Command_moveRight;
        _prevMoment = cmd.moment;
        break;
    }
}
