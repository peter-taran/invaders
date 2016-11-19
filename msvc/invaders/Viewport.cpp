#include "stdafx.h"
#include "Viewport.h"
#include "Sprite.h"
#include "GameConstants.h"


Viewport::Viewport():
    _consoleSize(),
    _consoleCharCount(0)
{
    _makeConsoleWindowPrivate();

    _checkConsoleHaveNoWin10Features();
}

Viewport::~Viewport()
{}

void Viewport::_makeConsoleWindowPrivate()
{
    array<DWORD, 2> pids = {};
    switch( GetConsoleProcessList(pids.data(), pids.size()) )
    {
    case 1: // we're only attached process
        break;
    default: // many processes, leaving them with their console
        FreeConsole();
        // no break here
    case 0: // we have no console window
        if( !AllocConsole() )
            throwGetLastError("allocating console");
        break;
    }
}

void Viewport::_checkConsoleHaveNoWin10Features()
{
    // Console features introduced in Windows 10 (also known as "version 2")
    // strongly interferes us
    // TODO implement this
}

Viewport::Console::Console(const DisplayCoords& size):
    handle{INVALID_HANDLE_VALUE}
{
    // TODO check Windows results everywhere

    handle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, nullptr,
        CONSOLE_TEXTMODE_BUFFER, nullptr);
    if( INVALID_HANDLE_VALUE == handle )
        throwGetLastError("creating console character buffer");
        
    if( !SetConsoleOutputCP(CP_UTF8) )
        throwGetLastError("setting UTF-8 encoding to console");

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if( !GetConsoleScreenBufferInfo(handle, &csbi) )
        throwGetLastError("retrieving console parameters");

    SMALL_RECT rect;
    zeroVar(rect);
    rect.Right = size.x - 1;
    rect.Bottom = size.y - 1;
    if( !SetConsoleScreenBufferSize(handle, size) ) // it's first
        throwGetLastError("resizing console");
    if( !SetConsoleWindowInfo(handle, TRUE, &rect) ) // it's second
        throwGetLastError("resizing console window");

    //CONSOLE_SCREEN_BUFFER_INFOEX csbi;
    //zeroVar(csbi);
    //csbi.cbSize = sizeof(csbi);
    //if( !GetConsoleScreenBufferInfoEx(handle, &csbi) )
    // ...
    //csbi.dwMaximumWindowSize = csbi.dwSize = size;
    //csbi.srWindow.Right = size.x - 1;
    //csbi.srWindow.Bottom = size.y - 1;
    //if( !SetConsoleScreenBufferInfoEx(handle, &csbi) )
    // ...
    //if( !GetConsoleScreenBufferInfoEx(handle, &csbi) )
    // ...
    
    CONSOLE_CURSOR_INFO cci;
    zeroVar(cci);
    cci.bVisible = FALSE;
    cci.dwSize = 1;
    SetConsoleCursorInfo(handle, &cci);

    if( !SetConsoleActiveScreenBuffer(handle) )
        throwGetLastError("switching screen buffer");

    if( !SetConsoleTitle(L"Морсеане отакуют!") )
        throwGetLastError("changing console window title");
}

Viewport::Console::~Console()
{
    if( INVALID_HANDLE_VALUE != handle )
    {
        SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
        CloseHandle(handle);
    }
}

void Viewport::gameMode(const DisplayCoords& size)
{
    if( _console )
        return;

    _consoleSize = size;
    _consoleCharCount = _consoleSize.x * _consoleSize.y;
    allocZeroedArray(_drawBuff, _consoleCharCount);

    _console.reset(new Console(size));
}

void Viewport::stdMode()
{
    _console.reset();
}

void Viewport::eraseDrawFrame()
{
    CHAR_INFO ci;
    zeroVar(ci);
    ci.Char.UnicodeChar = L' ';
    ci.Attributes = CharAttr();
    //ci.Attributes = _bufferSwitcher == 0 ? CharAttr() : CharAttr(DisplayColor_black, DisplayColor_aqua);

    std::fill(_drawBuff.get(), _drawBuff.get()+_consoleCharCount, ci);
}

void Viewport::switchFrame()
{
    SMALL_RECT allRect;
    allRect.Left = 0;
    allRect.Top = 0;
    allRect.Right = _consoleSize.x - 1;
    allRect.Bottom = _consoleSize.y - 1;

    WriteConsoleOutput(_console->handle, _drawBuff.get(), _consoleSize,
        DisplayCoords(), &allRect);
}

CHAR_INFO* Viewport::_drawCharPtr(const int x, const int y)
{
    return &_drawBuff[x + y *_consoleSize.x];
}

void Viewport::draw(const DisplayCoords& at, const Sprite& sprite)
{
    // TODO clipping

    const DisplayCoords spriteSize = sprite.size();
    
    const CHAR_INFO* spriteChar = &sprite.chars();
    for(int row = 0; row < spriteSize.y; ++row)
    {
        CHAR_INFO* putChar = _drawCharPtr(at.x, at.y + row);
        for(int col = 0; col < spriteSize.x; ++col, ++putChar, ++spriteChar)
        {
            // mixing
            if( spriteChar->Char.UnicodeChar != L'\0' )
                *putChar = *spriteChar;
        }
    }
}

void Viewport::fillAttr(const CharAttr& attr, const DisplayRect& rect)
{
    // TODO clipping

    for(int y = rect.tl.y; y < rect.br.y; ++y)
    {
        CHAR_INFO* putChar = _drawCharPtr(rect.tl.x, y);
        for(int x = rect.tl.x; x < rect.br.x; ++x, ++putChar)
            putChar->Attributes = attr;
    }
}

void Viewport::drawTextLine(const wstring& text, const DisplayCoords& startAt)
{
    // TODO clipping

    CHAR_INFO* putChar = _drawCharPtr(startAt.x, startAt.y);
    foreach(wchar_t ch, text)
    {
        putChar->Char.UnicodeChar = ch;
        ++putChar;
    }
}
