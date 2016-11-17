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
    L"  ((ºº))  ",
    L"[==••••==]",
};

Gun::Gun(InputProcessor& input, const DisplayRect& area):
    TimeEater(willBeInitedLater),

    _positionY(area.tl.y),
    _xrange(),

    _stopMotionAtX(-1),
    _positionX(),
    _motion(),

    _image(IMAGE),

    _shooting(false),
    _imageShooting(IMAGE, CharAttr(DisplayColor_lightRed, DisplayColor_black))
{
    input.listenGunMoveModeChange(bind(&Gun::commandMove, this, _1));
    input.listenGunFireModeChange(bind(&Gun::commandFire, this, _1));

    const double gunHalfWidth = _image.size().x / 2;

    _xrange = std::make_pair(area.tl.x, area.br.x);
    _xrange.first   += gunHalfWidth;
    _xrange.second  -= gunHalfWidth + 2;
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
    viewport.draw(DisplayCoords(_positionX - 4, _positionY),
        _shooting ? _imageShooting : _image);
}

void Gun::commandFire(const Command<InputController::FireMode>& state)
{
    eatTimeUpTo(state.moment); // TODO how can we avoid manual call?

    // TODO real fire
    _shooting = state.now.opened;
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
}
