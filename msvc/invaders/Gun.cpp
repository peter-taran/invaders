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
    TimeEater(willBeInitedLater),

    _positionY(),
    _xrange(),

    _stopMoveX(-1),
    _positionX(),
    _movement(),
    _speed(),

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
    viewport.draw(DisplayCoord(_positionX - 4, _positionY),
        _shooting ? _imageShooting : _image);
}

void Gun::commandFire(const Command<InputController::FireMode>& state)
{
    eatTimeUpTo(state.moment); // TODO: how can we avoid manual call?

    // TODO: real fire
    _shooting = state.now.opened;
}

void Gun::commandMove(const Command<InputController::MoveMode>& state)
{
    eatTimeUpTo(state.moment); // TODO: how can we avoid manual call?

    int newMovement = state.now.direction;
    if( state.now.moveToRequested )
    {
        // ignoring small moves
        const double stopMoveX = dispToPhys(state.now.moveTo);

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

void Gun::eatTime(const double prevMoment, const double now)
{
    if( _movement == 0 )
        return;
    
    // uniform component of motion
    _positionX += (_movement * _speed) * (now - prevMoment);

    // accelerated component of motion
    if( _speed < SPAAG_MAX_SPEED )
    {
        const double speedUpRestPerc = (SPAAG_MAX_SPEED - _speed) / SPAAG_MAX_SPEED;
        const double speedUpMoment = prevMoment + (SPAAG_SPEEDUP_TIME * speedUpRestPerc);
        const double speedUpPeriod = (std::min)(speedUpMoment, now) - prevMoment;

        double speedAdd =
            (SPAAG_MAX_SPEED / SPAAG_SPEEDUP_TIME) // acceleration
            * speedUpPeriod;
        _speed += speedAdd;

        if( _movement < 0 )
            speedAdd = -speedAdd;
        _positionX += // a * t^2 / 2
            speedAdd * speedUpPeriod / 2;
    }

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
