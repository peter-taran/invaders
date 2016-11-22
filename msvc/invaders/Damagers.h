#pragma once

#include "Kinetic.h"


class Bomb: public TransientDrawable, public TimeEater, public Smartobject
{
public:
    ~Bomb();

protected:
    friend Smartobject; // create objects with Smartobject::create
    Bomb(const double droppedAtX, const double droppedAtY, const double moment);

private:
    double _posX = 0, _posY = 0;
    Motion1D _motion;

    virtual void eatTime(const double from, const double to);
    virtual bool timeToDie();
    virtual void drawYourself(class Viewport& viewport);
};


class GunShell: public TransientDrawable, public TimeEater, public Smartobject
{
public:
    ~GunShell();

protected:
    friend Smartobject; // create objects with Smartobject::create
    GunShell(const double shotAtX, const double shotAtY, const double moment);

private:
    double _posX = 0, _posY = 0;
    Motion1D _motion;

    virtual void eatTime(const double from, const double to);
    virtual bool timeToDie();
    virtual void drawYourself(class Viewport& viewport);
};

