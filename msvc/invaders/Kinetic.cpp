#include "stdafx.h"
#include "Kinetic.h"


Motion1D::~Motion1D()
{}

MotionWalls1D::MotionWalls1D(Motion1D* motion):
    _motion(motion),
    _wallMin(-HUGE_VAL),
    _wallMax(HUGE_VAL),
    _lastPos()
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

double MotionWalls1D::pointAt(const double moment)
{
    if( _motion )
        _lastPos = _motion->pointAt(moment);
    
    if( _lastPos < _wallMin )
    {
        _motion.reset();
        _lastPos = _wallMin;
    }
    else if( _lastPos > _wallMax )
    {
        _motion.reset();
        _lastPos = _wallMax;
    }
    
    return _lastPos;
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

double AcceleratedMotion1D::pointAt(const double moment)
{
    double result = _startPoint;

    // accelerated motion, x = a * t^2 / 2
    const double t = (std::min)(moment, _maxSpeedMoment) - _startMoment;
    result += _acceleration * t * t / 2;

    if( moment > _maxSpeedMoment )
    {
        // uniform motion after achieving max speed
        result += (moment - _maxSpeedMoment) * _maxSpeed;
    }

    return result;
}
