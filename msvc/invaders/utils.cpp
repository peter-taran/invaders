#include "stdafx.h"


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
