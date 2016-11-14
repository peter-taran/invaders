#pragma once

#include "Input.h"


class Displayable: noncopyable
{
public:
    Displayable();
    virtual ~Displayable();

    virtual void drawYourself(class Viewport& viewport);
    virtual void updateStateByTime(const double moment);
};

// collection of Displayable objects
class DisplayLayer: noncopyable
{
public:
    DisplayLayer(); // empty
    ~DisplayLayer();

    void drawAll(class Viewport& viewport);

    void putObject(Displayable* object);

    void removeObject(Displayable* object);

private:
    typedef list<Displayable*> Objects;
    Objects _objects;
};
