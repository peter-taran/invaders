#pragma once


// Interface for classes calculating 1-dimentional coordinate of time
struct Motion1D
{
    virtual ~Motion1D();
    virtual double pointAt(const double moment) = 0;
    virtual int direction(const double moment) const = 0; // -1, 0, +1
};

class MotionWalls1D: public Motion1D
{
    scoped_ptr<Motion1D> _motion;
    double _wallMin;
    double _wallMax;
    double _lastPos;

public:
    MotionWalls1D(Motion1D* motion); // takes owneship, create with operator new

    void setWalls(const std::pair<double, double>& walls);
    void setMinWall(const double minWall = -HUGE_VAL); // no arg - reset wall
    void setMaxWall(const double maxWall = HUGE_VAL); // no arg - reset wall
    
    virtual double pointAt(const double moment); // override
    virtual int direction(const double moment) const; // override
};

class AcceleratedMotion1D: public Motion1D
{
    const double _startPoint;
    const double _startMoment;
    const double _maxSpeed;
    const double _acceleration;
    const double _maxSpeedMoment;

    // initial speed maybe later

public:
    AcceleratedMotion1D(const double startPoint, const double startMoment,
        const double maxSpeed, const double speedUpTime);
        // speedUpTime cannot be 0 or negative
        // maxSpeed can be any, including 0 and negative

    virtual double pointAt(const double moment); // override
    virtual int direction(const double moment) const; // override
};
