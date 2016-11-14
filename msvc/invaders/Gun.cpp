#include "stdafx.h"
#include "Gun.h"
#include "Viewport.h"
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

Gun::Gun(InputProcessor& input, const DisplayCoord& viewportSize):
    _positionY(0),
    _xrange(),

    _positionX(0),
    _speed(0),
    _movement(0),
    _prevMoment(0),

    _image(IMAGE),

    _shooting(false),
    _imageShooting(IMAGE, CharAttr(DisplayColor_lightRed, DisplayColor_black))
{
    input.listenGunMoveModeChange(bind(&Gun::_onChange_move, this, _1, _2, _3));
    input.listenGunFireModeChange(bind(&Gun::_onChange_fire, this, _1, _2, _3));

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
    viewport.draw(DisplayCoord(_positionX, _positionY),
        _shooting ? _imageShooting : _image);
}

void Gun::_onChange_fire(const double moment, const InputController::FireMode& was,
    const InputController::FireMode& now)
{
    // TODO: real fire
    _shooting = now.opened;
}

void Gun::_onChange_move(const double moment, const InputController::MoveMode& was,
    const InputController::MoveMode& now)
{
    _doMoving(moment);

    if( _movement != now.direction )
    {
        _movement = now.direction;
        _speed = 0;
    }
}

void Gun::_doMoving(const double now)
{
    if( _movement == 0 )
    {
        _prevMoment = now;
        return;
    }
    
    // uniform component of motion
    _positionX += (_movement * _speed) * (now - _prevMoment);

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

        if( _movement < 0 )
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
        _movement = 0;
    }
    else if( _positionX > _xrange.second )
    {
        _positionX = _xrange.second;
        _speed = 0;
        _movement = 0;
    }
}

void Gun::updateStateByTime(const double moment)
{
    __super::updateStateByTime(moment);
    _doMoving(moment);
}
