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

    _stopMoveX(-1),
    _positionX(0),
    _speed(0),
    _movement(0),
    _prevMoment(0),

    _image(IMAGE),

    _shooting(false),
    _imageShooting(IMAGE, CharAttr(DisplayColor_lightRed, DisplayColor_black))
{
    input.listenGunMoveModeChange(bind(&Gun::commandMove, this, _1));
    input.listenGunFireModeChange(bind(&Gun::commandFire, this, _1));

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
    viewport.draw(DisplayCoord(_positionX - 5, _positionY),
        _shooting ? _imageShooting : _image);
}

void Gun::commandFire(const Command<InputController::FireMode>& state)
{
    // TODO: real fire
    _shooting = state.now.opened;
}

void Gun::commandMove(const Command<InputController::MoveMode>& state)
{
    _doMoving(state.moment);

    int newMovement = state.now.direction;
    if( state.now.moveToRequested )
    {
        // ignoring small moves
        const double stopMoveX = static_cast<double>(state.now.moveTo + 0.5);

        const double moveShift = stopMoveX - _positionX;
        if( abs(moveShift) < 1 )
        {
            state.now.moveToRequested = false;
        }
        else
        {
            newMovement = moveShift < 0 ? -1 : +1;
            _stopMoveX = stopMoveX;
        }
    }

    if( !state.now.moveToRequested )
    {
        _stopMoveX = newMovement > 0 ? 1000000 : -1000000;
    }

    if( _movement != newMovement )
    {
        _movement = newMovement;
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
    pair<double, double> xrange = _xrange;
    if( _movement < 0 )
    {
        if( xrange.first < _stopMoveX )
            xrange.first = _stopMoveX;
    }
    else
    {
        if( xrange.second > _stopMoveX )
            xrange.second = _stopMoveX;
    }

    if( _positionX < xrange.first )
    {
        _positionX = xrange.first;
        _speed = 0;
        _movement = 0;
    }
    else if( _positionX > xrange.second )
    {
        _positionX = xrange.second;
        _speed = 0;
        _movement = 0;
    }
}

void Gun::updateStateByTime(const double moment)
{
    __super::updateStateByTime(moment);
    _doMoving(moment);
}
