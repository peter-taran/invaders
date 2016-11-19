#pragma once

#include "Sprite.h"
#include "Input.h"
#include "Kinetic.h"


// self-propelled anti-aircraft gun «Äþáåëü» ("Dubel")
class Gun: public Drawable, public TimeEater, noncopyable
{
public:
    Gun(struct GameControllers& controllers, const DisplayRect& area);
    ~Gun();

    static int height(); // gun's height on display

    virtual void drawYourself(class Viewport& viewport); // override
    virtual void eatTime(const double from, const double to); // override

    // control actions
    void commandFire(const Command<InputController::FireMode>& state);
    void commandMove(const Command<InputController::MoveMode>& state);

private:
    struct GameControllers& _controllers;

    double _positionY;
    pair<double, double> _xrange;

    // current state
    double _stopMotionAtX;
    double _positionX;
    MotionWalls1D _motion;

    // picture
    Sprite _image;

    int _shootNo = 0;
    double _shootMoment = 0;
    void _shoot(const double moment);
    void _shootBarrel(const double moment, int barrelNo);
    void _autoShootIfTime(const double moment);

    // TODO temp, to debug
    Sprite _imageShooting;
};
