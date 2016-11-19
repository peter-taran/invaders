#include "stdafx.h"
#include "Damagers.h"
#include "GameConstants.h"
#include "Viewport.h"


Bomb::Bomb(const double droppedAtX, const double droppedAtY, const double moment):
    TimeEater(moment)
{
    _motion = AcceleratedMotion1D(
        droppedAtY, moment, BOMB_MAX_SPEED, BOMB_SPEED_UP_TIME
    );
    _motion.updatePoint(_posY, moment);

    _posX = droppedAtX;
}

Bomb::~Bomb()
{}

void Bomb::eatTime(const double from, const double to)
{
    _motion.updatePoint(_posY, to);
}

void Bomb::drawYourself(Viewport& viewport)
{
    viewport.drawTextLine(L"o", {_posX, _posY});
}

bool Bomb::timeToDie()
{
    return _posY >= 35; // TODO implement!
}
