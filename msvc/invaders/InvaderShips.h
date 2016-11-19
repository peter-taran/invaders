#pragma once

#include "Sky.h"
#include "Sprite.h"
#include "Kinetic.h"


class InvaderShip: public TransientDrawable, public TimeEater, noncopyable
{
public:
    InvaderShip(const double moment);
    ~InvaderShip();
};

class RegularBomber: public InvaderShip
{
public:
    RegularBomber(struct GameControllers& controllers, const double moment,
        Echelon& echelon, int direction);
        // -1: left-to-right; +1: right-to-left
    ~RegularBomber();

    virtual void drawYourself(class Viewport& viewport);
    virtual void eatTime(const double from, const double to);
    virtual bool timeToDie();

private:
    Echelon& _echelon;
    struct GameControllers& _controllers;
    Sprite _image;
    pair<double, double> _xrange;
    double _posX = 0;
    double _posY = 0;
    Motion1D _motion;
    double _dropPoint = 0;
    bool _bombDropped = false;

    bool _timeToBombSaddam(const double moment);
};

