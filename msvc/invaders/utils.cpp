#include "stdafx.h"


timers::Now g_now;

boost::random::mt19937 g_random;

struct UnreachableException: public std::exception
{
    UnreachableException(const char* codePlaceID):
        std::exception((string("impossible code point executed: ") + codePlaceID).c_str())
    {}
};

void unreachable(const char* codePlaceID)
{
    throw UnreachableException(codePlaceID);
}
