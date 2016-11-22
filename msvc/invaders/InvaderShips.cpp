#include "stdafx.h"
#include "InvaderShips.h"
#include "Viewport.h"
#include "Game.h"
#include "Damagers.h"


InvaderShip::InvaderShip(const double moment):
    TimeEater(moment)
{}

InvaderShip::~InvaderShip()
{}

static const array<wstring, 2> REGULAR_BOMBER_IMG
{
    L"_______",
    L"\\== ==/",
};
static const DisplayCoords REGULAR_BOMBER_BOMB_POINT {3, 1};

RegularBomber::RegularBomber(GameControllers& controllers, const double moment,
    Echelon& echelon, int direction)
:
    InvaderShip(moment),
    _controllers{controllers},
    _echelon{echelon},
    _image{REGULAR_BOMBER_IMG},
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

    _posY = _echelon.area().br.y - _image.size().y;
        
    _dropPoint =
        uniform_real_distribution<double>(_xrange.first, _xrange.second)(g_random);
}

RegularBomber::~RegularBomber()
{}

void RegularBomber::drawYourself(Viewport& viewport)
{
    viewport.draw(
        DisplayCoords{physToDisp(_posX), physToDisp(_posY)},
        _image);

    viewport.drawTextLine(_bombDropped ? L" " : L"o",
        DisplayCoords{REGULAR_BOMBER_BOMB_POINT.x + _posX,
                      REGULAR_BOMBER_BOMB_POINT.y + _posY});
}

void RegularBomber::eatTime(const double from, const double to)
{
    _motion.updatePoint(_posX, to);

    if( _timeToBombSaddam(to) )
    {
        shared_ptr<Bomb> bomb = Smartobject::create<Bomb>(
            _posX + REGULAR_BOMBER_BOMB_POINT.x, _posY + REGULAR_BOMBER_BOMB_POINT.y + 1,
            to
        );
        _controllers.transients.put(bomb);
        _controllers.timeEaters.put(bomb);
        _bombDropped = true;
    }
}

bool RegularBomber::timeToDie()
{
    return _posX < _xrange.first || _posX > _xrange.second;
}

bool RegularBomber::_timeToBombSaddam(const double moment)
{
    if( _bombDropped )
        return false;
    return _motion.direction(moment) < 0 ? _posX <= _dropPoint : _posX >= _dropPoint;
}
