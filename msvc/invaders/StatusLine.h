#pragma once


class StatusLine: public Drawable
{
public:
    StatusLine(const DisplayCoord& viewportSize);
    ~StatusLine();

    virtual void drawYourself(class Viewport& viewport); // override

private:
    int _charCount;
    int _bottomIndex;
};

