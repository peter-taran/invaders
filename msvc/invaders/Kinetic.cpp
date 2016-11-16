#include "stdafx.h"
#include "Kinetic.h"


// all NoMotion objects share the same implementation
NoMotion::Impl NoMotion::impl;

Motion1D::Interface::~Interface()
{}

bool Motion1D::isNoMotion() const
{
    return &NoMotion::impl == _pimpl;
}

Motion1D::Motion1D(Interface* pimpl):
    _pimpl(pimpl)
{}

Motion1D::Motion1D():
    _pimpl(&NoMotion::impl)
{}

Motion1D::~Motion1D()
{
    if( _pimpl != &NoMotion::impl )
        delete _pimpl;
}

Motion1D::Motion1D(Motion1D&& from):
    _pimpl(from._pimpl)
{
    from._pimpl = nullptr;
}

Motion1D& Motion1D::operator=(Motion1D&& from)
{
    (std::swap)(_pimpl, from._pimpl);
    return *this;
}

Motion1D::Motion1D(const Motion1D& from):
    _pimpl(from._pimpl->clone())
{}

Motion1D& Motion1D::operator=(const Motion1D& from)
{
    if( &from != this )
    {
        this->~Motion1D();
        new(this) Motion1D(from);
    }
    return *this;
}

void Motion1D::updatePoint(double& point, const double moment)
{
    _pimpl->updatePoint(point, moment);
}

int Motion1D::direction(const double moment) const
{
    return _pimpl->direction(moment);
}

NoMotion::NoMotion():
    Motion1D(&NoMotion::impl)
{}

NoMotion::Interface* NoMotion::Impl::clone()
{
    return this;
}

void NoMotion::Impl::updatePoint(double& point, const double moment)
{}

int NoMotion::Impl::direction(const double moment) const
{
    return 0;
}

MotionWalls1D::MotionWalls1D():
    Motion1D(new Impl) // TODO: fix it
{}

MotionWalls1D::MotionWalls1D(Motion1D&& motion):
    Motion1D(new Impl) // TODO: fix it with moved motion
{}

MotionWalls1D::MotionWalls1D(const Motion1D& motion):
    // TODO: fix it
    // Motion1D{new Impl{_motion, -HUGE_VAL, HUGE_VAL}}
    Motion1D(new Impl)
{}

MotionWalls1D::Interface* MotionWalls1D::Impl::clone()
{
    return new Impl(*this);
}

int MotionWalls1D::Impl::direction(const double moment) const
{
    return _motion.direction(moment);
}

void MotionWalls1D::Impl::updatePoint(double& point, const double moment)
{
    if( _motion.isNoMotion() ) // some optimization
        return;
    
    _motion.updatePoint(point, moment);
    
    if( point < _wallMin )
    {
        _motion = NoMotion();
        point = _wallMin;
    }
    else if( point > _wallMax )
    {
        _motion = NoMotion();
        point = _wallMax;
    }
}

void MotionWalls1D::setWalls(const std::pair<double, double>& walls)
{
    Impl& data = impl<Impl>();
    data._wallMin = walls.first;
    data._wallMax = walls.second;
}

void MotionWalls1D::setMinWall(const double minWall)
{
    impl<Impl>()._wallMin = minWall;
}

void MotionWalls1D::setMaxWall(const double maxWall)
{
    impl<Impl>()._wallMax = maxWall;
}

AcceleratedMotion1D::AcceleratedMotion1D(const double startPoint,
    const double startMoment, const double maxSpeed, const double speedUpTime)
:
    Motion1D(new Impl) // TODO: all parameters
    /*_startPoint(startPoint),
    _startMoment(startMoment),
    _maxSpeed(maxSpeed),
    _acceleration(maxSpeed / speedUpTime),
    _maxSpeedMoment(startMoment + speedUpTime)*/
{
    assert(speedUpTime > 0);
}

AcceleratedMotion1D::Interface* AcceleratedMotion1D::Impl::clone()
{
    return new Impl(*this);
}

int AcceleratedMotion1D::Impl::direction(const double) const
{
    if( _maxSpeed == 0 )
        return 0;
    else if( _maxSpeed < 0 )
        return -1;
    else
        return +1;
}

void  AcceleratedMotion1D::Impl::updatePoint(double& point, const double moment)
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
