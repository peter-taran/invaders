#pragma once

#include "GameConstants.h"


// Sky echelon for horizontal flying
class Echelon
{
public:
    Echelon(const DisplayRect& area);
    ~Echelon();

    const DisplayRect& area();

private:
    DisplayRect _area;
};

class Sky
{
public:
    Sky(const DisplayRect& area);
    ~Sky();

    static int height();

    pair<Echelon&, bool> randomLowEchelon();
        // second: odd or even

private:
    array<scoped_ptr<Echelon>, ECHELONG_LOW_COUNT> _echelonsLow;
    array<scoped_ptr<Echelon>, ECHELONG_HIGH_COUNT> _echelonsHigh;
};

