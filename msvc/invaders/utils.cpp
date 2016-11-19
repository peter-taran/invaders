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

struct Win32Error: public std::exception
{
    Win32Error(const char* atWhat, DWORD win32ErrorCode = GetLastError()):
        std::exception(messageOf(atWhat, win32ErrorCode).c_str())
    {}

    static string messageOf(const char* atWhat, DWORD win32ErrorCode)
    {
        if( !atWhat || atWhat[0] == '\0' )
            atWhat = "(unidentified code point)";

        if( 0 == win32ErrorCode )
        {
            return (format("No error at %1%. Mistakenly thrown exception")
                % atWhat)
                .str();
        }
        else
        {
            LPSTR win32ErrorText = nullptr;
            size_t win32ErrorTextLen = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, win32ErrorCode, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                (LPSTR)&win32ErrorText, 0, NULL);

            string res = (format("Error at %1%. %2%")
                % atWhat
                % win32ErrorText)
                .str();
            size_t last = res.length();
            for(; last > 0; --last)
            {
                const char c = res.at(last - 1);
                if( c != '\r' && c != '\n' && c != '.' && c != ' ' )
                    break;
                    
            }
            res.erase(last);

            LocalFree(win32ErrorText);

            return res;
        }
    }
};

void throwGetLastError(const char* atWhat)
{
    throw Win32Error(atWhat);
}
