#include "stdafx.h"
#include "Basic.h"


TimeEater::TimeEater(WillBeInitedLater):
    _eatenUpTo(0.0)
{}

void TimeEater::laterInit(const double startMoment)
{
    assert(0.0 == _eatenUpTo);
    _eatenUpTo = startMoment;
}

TimeEater::TimeEater(const double startMoment):
    _eatenUpTo(startMoment)
{}

double TimeEater::eatenUpTo() const
{
    return _eatenUpTo;
}

void TimeEater::eatTimeUpTo(const double moment)
{
    if( _eatenUpTo == moment )
        return;
    eatTime(_eatenUpTo, moment);
    _eatenUpTo = moment;
}

DisplayAreaVertLayout::DisplayAreaVertLayout(const DisplayRect& totalArea):
    _rect(totalArea)
{}

void DisplayAreaVertLayout::gap(int gapY)
{
    _rect.br.y -= gapY;
}

DisplayRect DisplayAreaVertLayout::nextField(int height)
{
    DisplayRect ret = _rect;
    ret.tl.y = ret.br.y - height;
    _rect.br.y = ret.tl.y;
    return ret;
}

DisplayRect DisplayAreaVertLayout::restArea()
{
    return _rect;
}
