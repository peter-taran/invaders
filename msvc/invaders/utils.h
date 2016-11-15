#pragma once


inline void zeroBytes(void* startFrom, void* afterEnd)
{
    assert(afterEnd >= startFrom);
    memset(startFrom, 0, static_cast<char*>(afterEnd) - static_cast<char*>(startFrom));
}

template<class Type>
void zeroVar(Type& variable)
{
    compile_assert(boost::is_pod<Type>::value);
        // zeroing is for POD types only

    zeroBytes(&variable, &variable + 1);
}

template<class Type>
void allocZeroedObj(scoped_ptr<Type>& pointer)
{
    pointer.reset(new Type);
    zeroVar(*pointer);
}

template<class Type>
void allocZeroedArray(scoped_array<Type>& pointer, std::ptrdiff_t size)
{
    compile_assert(boost::is_pod<Type>::value);
        // zeroing is for POD types only

    pointer.reset(new Type[size]);
    zeroBytes(pointer.get(), pointer.get() + size);
}

void __declspec(noreturn) unreachable(const char* codePlaceID);

// timer to get current moment anywhere
extern timers::Now g_now;