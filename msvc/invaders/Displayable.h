#pragma once

#include "InputController.h"


class Displayable: noncopyable
{
public:
    Displayable();
    virtual ~Displayable();

    virtual void drawYourself(class Viewport& viewport);

    virtual void processCommand(const Command& cmd);
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
