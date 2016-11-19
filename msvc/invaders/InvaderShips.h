#pragma once

#include "Sky.h"
#include "Sprite.h"
#include "Kinetic.h"


class InvaderShip: public TransientDrawable, public TimeEater
{
public:
    InvaderShip(const double moment);
    ~InvaderShip();
};

class RegularBomber: public InvaderShip
{
public:
    RegularBomber(const double moment, Echelon& echelon, int direction);
        // -1: left-to-right; +1: right-to-left
    ~RegularBomber();

    virtual void drawYourself(class Viewport& viewport);
    virtual void eatTime(const double from, const double to);
    virtual bool timeToDie();

private:
    Echelon& _echelon;
    Sprite _image;
    pair<double, double> _xrange;
    double _posX;
    Motion1D _motion;
};

