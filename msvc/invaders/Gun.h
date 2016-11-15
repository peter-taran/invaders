#pragma once

#include "Sprite.h"
#include "Input.h"


// self-propelled anti-aircraft gun «Äþáåëü» ("Dubel")
class Gun: public Drawable, public TimeEater
{
public:
    Gun(InputProcessor& input, const DisplayCoord& viewportSize);
    ~Gun();

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
    double _stopMoveX;
    double _positionX;
    int _movement;
    double _speed;
    void _doMoving(const double now);

    // picture
    Sprite _image;

    // TODO: temp, to debug
    bool _shooting;
    Sprite _imageShooting;
};
