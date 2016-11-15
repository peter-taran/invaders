#include "stdafx.h"
#include "Basic.h"


TimeEater::TimeEater(WillBeInitedLater):
    _eatenUpTo(0.0)
{}

void TimeEater::laterInit(const double& startMoment)
{
    assert(0.0 == _eatenUpTo);
    _eatenUpTo = startMoment;
}

TimeEater::TimeEater(const double& startMoment):
    _eatenUpTo(startMoment)
{}

double TimeEater::eatenUpTo() const
{
    return _eatenUpTo;
}

void TimeEater::eatTimeUpTo(const double& moment)
{
    if( _eatenUpTo == moment )
        return;
    eatTime(_eatenUpTo, moment);
    _eatenUpTo = moment;
}
