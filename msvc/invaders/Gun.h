#pragma once

#include "Displayable.h"
#include "Sprite.h"


// self-propelled anti-aircraft gun
class Gun: public Displayable
{
public:
    Gun(const DisplayCoord& viewportSize);
    ~Gun();

    virtual void drawYourself(class Viewport& viewport); // override
    virtual void processCommand(const Command& cmd); // override

private:
    // consts
    double _positionY;
    pair<double, double> _xrange;

    // current state
    double _positionX;
    CommandType _movement;
    double _speed;
    double _prevMoment;
    void _doMoving(const double now);

    // picture
    Sprite _image;
};
