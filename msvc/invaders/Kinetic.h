#pragma once


// 1-dimensional motion controller
// 
class Motion1D
{
public: // for inheritors only; inheritors must not add members
    struct Interface
    {
        bool isNoMotion() const; // for optimizations
        virtual ~Interface(); // for safe deletion

        // implement it
        virtual Interface* clone() = 0; // makes a copied clone with operator new
        virtual void updatePoint(double& point, const double moment) = 0;
        virtual int direction(const double moment) const = 0;
    };

protected:
    Interface* _pimpl;
    explicit Motion1D(Interface* pimpl);
    template<class Impl> const Impl& impl() const
        { return static_cast<const Impl&>(*_pimpl); }
    template<class Impl> Impl& impl()
        { return static_cast<Impl&>(*_pimpl); }
    
public:
    Motion1D(); // no motion by default
    virtual ~Motion1D();

    // you can copy or move Motion1D -> Motion1D or inheritor to Motion1D
    // like this:
    //   Motion1D m1;
    //   Motion1D m2 = UniformMotion1D(momentNow, speed);
    //   m1 = m2;
    //   m2 = NoMotion();
    Motion1D(Motion1D&& from);
    Motion1D& operator=(Motion1D&& from);
    Motion1D(const Motion1D& from);
    Motion1D& operator=(const Motion1D& from);

    // updates value of coordinate point for "current" moment
    // previous value of point commonly ignored, excepting no motion case
    void updatePoint(double& point, const double moment);

    // what direction point moves at moment (~ sign of speed):
    //   -1: decreasing
    //    0: stays
    //   +1: increasing
    int direction(const double moment) const;
};

// implement all motion interfaces, but goes nowhere
class NoMotion: public Motion1D
{
    struct Impl: Motion1D::Interface
    {
        virtual Interface* clone();
        virtual void updatePoint(double& point, const double moment);
        virtual int direction(const double moment) const;
    };
    static Impl impl;
    friend Motion1D;

public:
    NoMotion();
};

// wrapper about another motion giving ability to stop it when
// point goes outside of min/max walls
class MotionWalls1D: public Motion1D
{
    struct Impl: Motion1D::Interface
    {
        Motion1D _motion;
        double _wallMin;
        double _wallMax;

        virtual Interface* clone();
        virtual void updatePoint(double& point, const double moment);
        virtual int direction(const double moment) const;
    };

public:
    MotionWalls1D();
    
    // that's not copy constructors, motion argument is motion to be wrapped;
    // not explicit keyword because it's no walls by default and this behaves
    // same as motion was copied
    MotionWalls1D(const Motion1D& motion);
    MotionWalls1D(Motion1D&& motion);

    void setWalls(const std::pair<double, double>& walls);
    void setMinWall(const double minWall = -HUGE_VAL); // no arg - reset wall
    void setMaxWall(const double maxWall = HUGE_VAL); // no arg - reset wall
};

class AcceleratedMotion1D: public Motion1D
{
    struct Impl: Motion1D::Interface
    {
        // TODO: add consts
        double _startPoint;
        double _startMoment;
        double _maxSpeed;
        double _acceleration;
        double _maxSpeedMoment;
        // initial speed maybe later

        virtual Interface* clone();
        virtual void updatePoint(double& point, const double moment);
        virtual int direction(const double moment) const;
    };

public:
    AcceleratedMotion1D(const double startPoint, const double startMoment,
        const double maxSpeed, const double speedUpTime);
        // speedUpTime cannot be 0 or negative
        // maxSpeed can be any, including 0 and negative
};
