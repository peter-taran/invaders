#include "stdafx.h"
#include "Kinetic.h"


MotionController1D::~MotionController1D()
{}

// implement all motion interfaces, but goes nowhere
class NoMotion: public MotionController1D
{
    virtual void updatePoint(double& point, const double moment)
    {}

    virtual int direction(const double moment) const
    {
        return 0;
    }
};
static NoMotion g_noMotion;

MotionWalls1D::MotionWalls1D(MotionController1D* motion):
    _motion(motion),
    _wallMin(-HUGE_VAL),
    _wallMax(HUGE_VAL)
{}

void MotionWalls1D::setWalls(const std::pair<double, double>& walls)
{
    _wallMin = walls.first;
    _wallMax = walls.second;
}

void MotionWalls1D::setMinWall(const double minWall)
{
    _wallMin = minWall;
}

void MotionWalls1D::setMaxWall(const double maxWall)
{
    _wallMax = maxWall;
}

int MotionWalls1D::direction(const double moment) const
{
    return _motion ? _motion->direction(moment) : 0;
}

void  MotionWalls1D::updatePoint(double& point, const double moment)
{
    if( !_motion )
        return;
    
    _motion->updatePoint(point, moment);
    
    if( point < _wallMin )
    {
        _motion.reset();
        point = _wallMin;
    }
    else if( point > _wallMax )
    {
        _motion.reset();
        point = _wallMax;
    }
}

AcceleratedMotion1D::AcceleratedMotion1D(const double startPoint,
    const double startMoment, const double maxSpeed, const double speedUpTime)
:
    _startPoint(startPoint),
    _startMoment(startMoment),
    _maxSpeed(maxSpeed),
    _acceleration(maxSpeed / speedUpTime),
    _maxSpeedMoment(startMoment + speedUpTime)
{
    assert(speedUpTime > 0);
}

int AcceleratedMotion1D::direction(const double) const
{
    if( _maxSpeed == 0 )
        return 0;
    else if( _maxSpeed < 0 )
        return -1;
    else
        return +1;
}

void  AcceleratedMotion1D::updatePoint(double& point, const double moment)
{
    point = _startPoint;

    // accelerated motion, x = a * t^2 / 2
    const double t = (std::min)(moment, _maxSpeedMoment) - _startMoment;
    point += _acceleration * t * t / 2;

    if( moment > _maxSpeedMoment )
    {
        // uniform motion after achieving max speed
        point += (moment - _maxSpeedMoment) * _maxSpeed;
    }
}
