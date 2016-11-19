#pragma once

#include "Sprite.h"
#include "Input.h"
#include "Kinetic.h"


// self-propelled anti-aircraft gun «Äþáåëü» ("Dubel")
class Gun: public Drawable, public TimeEater
{
public:
    Gun(const DisplayRect& area);
    ~Gun();

    static int height(); // gun's height on display

    virtual void drawYourself(class Viewport& viewport); // override
    virtual void eatTime(const double from, const double to); // override

    // control actions
    void commandFire(const Command<InputController::FireMode>& state);
    void commandMove(const Command<InputController::MoveMode>& state);

private:
    // consts
    double _positionY;
    pair<double, double> _xrange;

    // current state
    double _stopMotionAtX;
    double _positionX;
    MotionWalls1D _motion;

    // picture
    Sprite _image;

    // TODO temp, to debug
    bool _shooting;
    Sprite _imageShooting;
};
