#include "stdafx.h"
#include "InvaderShips.h"
#include "Viewport.h"


InvaderShip::InvaderShip(const double moment):
    TimeEater(moment)
{}

InvaderShip::~InvaderShip()
{}

static const array<wstring, 2> IMG_REGULAR_BOMBER =
{
    L"_______",
    L"\\==o==/",
};

RegularBomber::RegularBomber(const double moment, Echelon& echelon, int direction):
    InvaderShip(moment),
    _echelon{echelon},
    _image{IMG_REGULAR_BOMBER},
    _xrange{
        dispToPhys(echelon.area().tl.x),
        dispToPhys(echelon.area().br.x - _image.size().x)},
    _posX{}
{
    assert(direction == -1 || direction == +1);

    _motion = UniformMotion1D{
        direction == -1 ? _xrange.first : _xrange.second,
        moment,
        direction == -1 ? +SHIP_REGUALAR_SPEED : -SHIP_REGUALAR_SPEED
    };
    _motion.updatePoint(_posX, moment);
}

RegularBomber::~RegularBomber()
{}

void RegularBomber::drawYourself(Viewport& viewport)
{
    viewport.draw(
        DisplayCoords{physToDisp(_posX), _echelon.area().br.y - _image.size().y},
        _image);
}

void RegularBomber::eatTime(const double from, const double to)
{
    _motion.updatePoint(_posX, to);
}

bool RegularBomber::flewAway()
{
    return _posX < _xrange.first || _posX > _xrange.second;
}
