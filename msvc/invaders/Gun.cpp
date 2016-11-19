#include "stdafx.h"
#include "Gun.h"
#include "Viewport.h"
#include "GameConstants.h"
#include "Damagers.h"
#include "Game.h"


/*static const array<wstring, 3> IMAGE = {
    L"    ||    ",
    L"  ((oo))  ",
    L"[==0000==]",
};*/
static const array<wstring, 3> IMAGE = {
    L"    !!    ",
    L"  ((ºº))  ",
    L"[==••••==]",
};
static const double MIDDLE_POINT_X = 5;
static const double LEFT_BARREL_X = 4;

Gun::Gun(struct GameControllers& controllers, const DisplayRect& area):
    TimeEater(willBeInitedLater),
    _controllers(controllers),

    _positionY(area.tl.y),
    _xrange(),

    _stopMotionAtX(-1),
    _positionX(),
    _motion(),

    _image(IMAGE),

    _imageShooting(IMAGE, CharAttr(DisplayColor_lightRed, DisplayColor_black))
{
    const double gunHalfWidth = _image.size().x / 2;

    _xrange = make_pair(area.tl.x, area.br.x);
    _xrange.first   += 1 + MIDDLE_POINT_X;
    _xrange.second  -= 1 + (_image.size().x - MIDDLE_POINT_X);
    _positionX = (_xrange.first + _xrange.second) / 2;
}

Gun::~Gun()
{}

int Gun::height()
{
    return IMAGE.static_size;
}

void Gun::drawYourself(Viewport& viewport)
{
    viewport.draw(DisplayCoords(_positionX - MIDDLE_POINT_X, _positionY), _image);
}

void Gun::commandFire(const Command<InputController::FireMode>& state)
{
    eatTimeUpTo(state.moment); // TODO how can we avoid manual call?

    if( state.now.opened )
    {
        if( _shootNo == 0 )
        {
            _shoot(state.moment);
        }
    }
    else
    {
        if( _shootNo > 0 )
            _shootNo = 0;
    }
}

void Gun::commandMove(const Command<InputController::MoveMode>& state)
{
    eatTimeUpTo(state.moment); // TODO how can we avoid manual call?

    int newDirection = state.now.direction;
    bool hasMotionBound = false;
    double motionBound = 0;

    if( state.now.moveToRequested )
    {
        motionBound = dispToPhys(state.now.moveTo);

        const double moveShift = motionBound - _positionX;
        if( abs(moveShift) < 1 )
        {
            // ignore small moves
            state.now.moveToRequested = false;
        }
        else
        {
            newDirection = moveShift < 0 ? -1 : +1;
            hasMotionBound = true;
        }
    }

    if( _motion.direction(state.moment) != newDirection )
    {
        if( 0 == newDirection)
        {
            _motion = NoMotion();
        }
        else
        {
            _motion = MotionWalls1D(
                AcceleratedMotion1D(
                    _positionX, state.moment,
                    (newDirection > 0 ? GUN_MAX_SPEED : -GUN_MAX_SPEED),
                    GUN_SPEEDUP_TIME
            ));
            _motion.setWalls(_xrange);
        }
    }

    if( hasMotionBound )
    {
        newDirection > 0
            ? _motion.setMaxWall((std::min)(motionBound, _xrange.second))
            : _motion.setMinWall((std::max)(motionBound, _xrange.first));
    }
}

void Gun::eatTime(const double prevMoment, const double now)
{
    _motion.updatePoint(_positionX, now);
    _autoShootIfTime(now);
}

static const double GUN_TIME_AFTER_FIRST = GUN_DIST_AFTER_FIRST / GUN_SHELL_SPEED;
static const double GUN_TIME_IN_BIRST = GUN_DIST_IN_BIRST / GUN_SHELL_SPEED;

void Gun::_autoShootIfTime(const double moment)
{
    bool timeToShootInvaders = false;
    if( _shootNo > 2 )
    {
        // automatic fire
        timeToShootInvaders = (moment >= _shootMoment + GUN_TIME_IN_BIRST);
    }
    else if( _shootNo > 0 )
    {
        // after first double-shoot
        timeToShootInvaders = (moment >= _shootMoment + GUN_TIME_AFTER_FIRST);
    }
    
    if( timeToShootInvaders )
        _shoot(moment);
}

void Gun::_shoot(const double moment)
{
    _shootMoment = moment;
    if( _shootNo == 0 )
    {
        _shootBarrel(moment, 0);
        _shootBarrel(moment, 1);
    }
    else
    {
        _shootBarrel(moment, _shootNo % 2);
    }
}

void Gun::_shootBarrel(const double moment, int barrelNo)
{
    ++_shootNo;
    const int dispX = physToDisp(_positionX - MIDDLE_POINT_X);
    shared_ptr<GunShell> shell {new GunShell{
        dispToPhys(dispX) + LEFT_BARREL_X + barrelNo,
        _positionY,
        moment
    }};
    _controllers.timeEaters.put(shell);
    _controllers.transients.put(shell);
}
