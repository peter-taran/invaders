#include "stdafx.h"
#include "Displayable.h"
#include "Viewport.h"


Displayable::Displayable()
{}

Displayable::~Displayable()
{}

void Displayable::drawYourself(Viewport& viewport)
{
    // nothing by default
}

void Displayable::processCommand(const Command& cmd)
{
    // nothing by default
}

DisplayLayer::DisplayLayer()
{}

DisplayLayer::~DisplayLayer()
{}

void DisplayLayer::drawAll(Viewport& viewport)
{
    foreach(Displayable* object, _objects)
    {
        if( object )
            object->drawYourself(viewport);
    }
}

void DisplayLayer::putObject(Displayable* object)
{
    if( !object )
        return; // it's strange, but not a problem

    _objects.push_back(object);
}

void DisplayLayer::removeObject(Displayable* object)
{
    _objects.remove(object);
}
