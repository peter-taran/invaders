#pragma once

#include "Displayable.h"
#include "Sprite.h"
#include "Input.h"


// self-propelled anti-aircraft gun
class Gun: public Displayable
{
public:
    Gun(InputProcessor& input, const DisplayCoord& viewportSize);
    ~Gun();

    virtual void drawYourself(class Viewport& viewport); // override
    virtual void updateStateByTime(const double moment); // override

private:
    // listeners
    void _onChange_fire(const double moment, const InputController::FireMode& was,
        const InputController::FireMode& now);
    void _onChange_move(const double moment, const InputController::MoveMode& was,
        const InputController::MoveMode& now);

    // consts
    double _positionY;
    pair<double, double> _xrange;

    // current state
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
