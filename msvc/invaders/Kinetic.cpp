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
    Motion1D(new Impl)
{}

MotionWalls1D::MotionWalls1D(Motion1D&& motion):
    Motion1D(new Impl)
{
    impl<Impl>().motion = std::move(motion);
}

MotionWalls1D::MotionWalls1D(const Motion1D& motion):
    Motion1D(new Impl)
{
    impl<Impl>().motion = motion;
}

MotionWalls1D::Interface* MotionWalls1D::Impl::clone()
{
    return new Impl(*this);
}

int MotionWalls1D::Impl::direction(const double moment) const
{
    return motion.direction(moment);
}

void MotionWalls1D::Impl::updatePoint(double& point, const double moment)
{
    if( motion.isNoMotion() ) // some optimization
        return;
    
    motion.updatePoint(point, moment);
    
    if( point < wallMin )
    {
        motion = NoMotion();
        point = wallMin;
    }
    else if( point > wallMax )
    {
        motion = NoMotion();
        point = wallMax;
    }
}

void MotionWalls1D::setWalls(const std::pair<double, double>& walls)
{
    Impl& data = impl<Impl>();
    data.wallMin = walls.first;
    data.wallMax = walls.second;
}

void MotionWalls1D::setMinWall(const double minWall)
{
    impl<Impl>().wallMin = minWall;
}

void MotionWalls1D::setMaxWall(const double maxWall)
{
    impl<Impl>().wallMax = maxWall;
}

AcceleratedMotion1D::AcceleratedMotion1D(const double startPoint,
    const double startMoment, const double maxSpeed, const double speedUpTime)
:
    Motion1D(new Impl)
{
    assert(speedUpTime > 0);

    // I hate to declare same constructors with looong parameter lists
    // both in AcceleratedMotion1D and it's Impl
    Impl& i = impl<Impl>();
    i.startPoint       = startPoint;
    i.startMoment      = startMoment;
    i.maxSpeed         = maxSpeed;
    i.acceleration     = maxSpeed / speedUpTime;
    i.maxSpeedMoment   = startMoment + speedUpTime;
}

AcceleratedMotion1D::Interface* AcceleratedMotion1D::Impl::clone()
{
    return new Impl(*this);
}

int AcceleratedMotion1D::Impl::direction(const double) const
{
    if( maxSpeed == 0 )
        return 0;
    else if( maxSpeed < 0 )
        return -1;
    else
        return +1;
}

void  AcceleratedMotion1D::Impl::updatePoint(double& point, const double moment)
{
    point = startPoint;

    // accelerated motion, x = a * t^2 / 2
    const double t = (std::min)(moment, maxSpeedMoment) - startMoment;
    point += acceleration * t * t / 2;

    if( moment > maxSpeedMoment )
    {
        // uniform motion after achieving max speed
        point += (moment - maxSpeedMoment) * maxSpeed;
    }
}

UniformMotion1D::UniformMotion1D(const double startPoint, const double startMoment,
    const double speed)
:
    Motion1D(new Impl)
{
    Impl& i = impl<Impl>();

    i.startPoint = startPoint;
    i.startMoment = startMoment;
    i.speed = speed;
}

UniformMotion1D::Interface* UniformMotion1D::Impl::clone()
{
    return new Impl{*this};
}

void UniformMotion1D::Impl::updatePoint(double& point, const double moment)
{
    point = startPoint + (moment - startMoment) * speed;
}

int UniformMotion1D::Impl::direction(const double moment) const
{
    return
        speed == 0 ?  0 :
        speed  < 0 ? -1 :
                     +1 ;
}
