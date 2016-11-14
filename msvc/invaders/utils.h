#pragma once


inline void zeroBytes(void* startFrom, void* afterEnd)
{
    assert(afterEnd >= startFrom);
    memset(startFrom, 0, static_cast<char*>(afterEnd) - static_cast<char*>(startFrom));
}

template<class Variable>
void zeroVar(Variable& variable)
{
    zeroBytes(&variable, &variable + 1);
}

void __declspec(noreturn) unreachable(const char* codePlaceID);

