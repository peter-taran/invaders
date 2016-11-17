#include "stdafx.h"
#include "SweetHome.h"
#include "Viewport.h"
#include "GameConstants.h"


/*
_______
\==o==/


     *
     *


     |
   ((*))
 <<=====>>
 
 ooooooooo


    O
   ^*^
  / | \
    =
   / \
  /   \

*/

static array<wstring, 6> IMG_HUMAN = {
    L"  O",
    L" ^*^",
    L"/ | \\",
    L"  =",
    L" / \\",
    L"/   \\",
};

static const Sprite SPRITE_HUMAN(IMG_HUMAN);

Human::Human(const DisplayCoords& pos):
    _pos(pos)
{}

Human::~Human()
{}

void Human::drawYourself(class Viewport& viewport)
{
    viewport.draw(_pos, SPRITE_HUMAN);
}

SweetHome::SweetHome(const DisplayRect& area):
    _posY(area.tl.y)
{
    _humans.reserve(HUMAN_NUMBER);

    const int stepX = SPRITE_HUMAN.size().x + HUMAN_DIST_BETWEEN;
    DisplayCoords pos(HUMAN_SPACE_LEFTRIGHT, _posY);
    for(int i = 0; i < HUMAN_NUMBER; ++i, pos.x += stepX)
    {
        _humans.push_back(Human(pos));
    }
}

SweetHome::~SweetHome()
{}

DisplayCoords SweetHome::minSize()
{
    const DisplayCoords hs = SPRITE_HUMAN.size();
    return DisplayCoords(
        HUMAN_NUMBER*(hs.x + HUMAN_DIST_BETWEEN) - HUMAN_DIST_BETWEEN
        + HUMAN_SPACE_LEFTRIGHT*2,
        hs.y
    );
}

void SweetHome::drawYourself(Viewport& viewport)
{
    foreach(Human& human, _humans)
    {
        human.drawYourself(viewport);
    }
}
