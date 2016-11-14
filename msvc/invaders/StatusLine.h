#pragma once

#include "Displayable.h"


class StatusLine: public Displayable
{
public:
    StatusLine(const DisplayCoord& viewportSize);
    ~StatusLine();

    virtual void drawYourself(class Viewport& viewport); // override
    virtual void processCommand(const Command& cmd); // override

private:
    int _charCount;
    int _bottomIndex;
};

