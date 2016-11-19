#pragma once

#include "Kinetic.h"


class Bomb: public TransientDrawable, public TimeEater
{
public:
    Bomb(const double droppedAtX, const double droppedAtY, const double moment);
    ~Bomb();

private:
    double _posX = 0, _posY = 0;
    Motion1D _motion;

    virtual void eatTime(const double from, const double to);
    virtual bool timeToDie();
    virtual void drawYourself(class Viewport& viewport);
};

