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

Gun::Gun(InputProcessor& input, const DisplayCoords& viewportSize):
    TimeEater(willBeInitedLater),

    _positionY(),
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

    const double gunWidth = _image.size().x;

    _xrange.first = (gunWidth / 2);
    _xrange.second = viewportSize.x - (gunWidth / 2) - 2;
    _positionX = (_xrange.first + _xrange.second) / 2;

    _positionY = 40; // TODO change to appropriate
}

Gun::~Gun()
{}

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
            _motion = NoMotion(); // TODO why it compiles?!
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
