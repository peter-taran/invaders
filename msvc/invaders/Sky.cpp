#include "stdafx.h"
#include "Sky.h"


Sky::Sky(const DisplayRect& area)
{
    DisplayAreaVertLayout layout = area;

    layout.gap(ECHELONG_NEVER_USED_SPACE);
    layout.gap(ECHELONG_KAMIKAZE_AREA_HEIGHT);

    for(scoped_ptr<Echelon>& e: _echelonsLow)
        e.reset(new Echelon{layout.nextField(ECHELONG_LOW_HEIGHT)});
    for(scoped_ptr<Echelon>& e: _echelonsHigh)
        e.reset(new Echelon{layout.nextField(ECHELONG_HIGH_HEIGHT)});
}

Sky::~Sky()
{}

int Sky::height()
{
    return
        ECHELONG_NEVER_USED_SPACE
      + ECHELONG_KAMIKAZE_AREA_HEIGHT
      + ECHELONG_LOW_HEIGHT * ECHELONG_LOW_COUNT
      + ECHELONG_HIGH_HEIGHT * ECHELONG_HIGH_COUNT;
}

pair<Echelon&, bool> Sky::randomLowEchelon()
{
    int index = uniform_int_distribution<int>(0, _echelonsLow.static_size-1)(g_random);
    return pair<Echelon&, bool>(*_echelonsLow.at(index), 0 == (index % 2));
}

Echelon::Echelon(const DisplayRect& area):
    _area(area)
{}

Echelon::~Echelon()
{}

const DisplayRect & Echelon::area()
{
    return _area;
}
