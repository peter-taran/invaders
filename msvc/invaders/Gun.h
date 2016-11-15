#pragma once

#include "Displayable.h"
#include "Sprite.h"
#include "Input.h"


// self-propelled anti-aircraft gun «Äþáåëü» ("Dubel")
class Gun: public Displayable
{
public:
    Gun(InputProcessor& input, const DisplayCoord& viewportSize);
    ~Gun();

    virtual void drawYourself(class Viewport& viewport); // override
    virtual void updateStateByTime(const double moment); // override

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
    double _prevMoment;
    void _doMoving(const double now);

    // picture
    Sprite _image;

    // TODO: temp, to debug
    bool _shooting;
    Sprite _imageShooting;
};
