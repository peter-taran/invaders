#pragma once


class StatusLine: public Drawable
{
public:
    StatusLine(const DisplayRect& area);
    ~StatusLine();

    static int height(); // how many lines on display it takes

    virtual void drawYourself(class Viewport& viewport); // override

private:
    int _charCount;
    int _bottomIndex;
};

