#pragma once

#include "Sprite.h"


// human to protect
class Human: public Drawable
{
public:
    Human(const DisplayCoords& pos);
    ~Human();

    void drawYourself(class Viewport& viewport);

private:
    DisplayCoords _pos;
    //Sprite _image;
};

// protecting area
class SweetHome: public Drawable
{
public:
    SweetHome(const DisplayRect& area);
    ~SweetHome();

    static DisplayCoords minSize(); // how much place we need

    void drawYourself(class Viewport& viewport);

private:
    typedef vector<Human> Humans;
    Humans _humans;

    int _posY;
};
