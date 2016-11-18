#pragma once


// Zen function doing absolutely nothing
template<class Type>
void doNothing(Type*)
{}

// timer to get current moment anywhere
extern timers::Now g_now;

// random generator
extern boost::random::mt19937 g_random;

// mark point should never reached
void __declspec(noreturn) unreachable(const char* codePlaceID);

// Declaration to say, that object should stay uninitialized (will be initialized later)
static const struct WillBeInitedLater {} willBeInitedLater;

// zero bytes in range
inline void zeroBytes(void* startFrom, void* afterEnd)
{
    assert(afterEnd >= startFrom);
    memset(startFrom, 0, static_cast<char*>(afterEnd) - static_cast<char*>(startFrom));
}

// zero POD variable contents
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

// cleanup garbage of container - remove deleted objects
template<class Type>
void cleanup(deque< weak_ptr<Type> >& elems)
{
    deque< weak_ptr<Type> > copied;
    foreach(const weak_ptr<Type>& elem, elems)
    {
        if( !elem.expired() ) // in multithreading some garbage can stay, it's OK
            copied.push_back(std::move(elem));
    }
    elems.swap(copied);
}

// cleanup garbage of container - null pointers
template<class Type>
void cleanup(deque< shared_ptr<Type> >& elems)
{
    deque< shared_ptr<Type> > copied;
    foreach(const shared_ptr<Type>& elem, elems)
    {
        if( elem )
            copied.push_back(std::move(elem));
    }
    elems.swap(copied);
}
