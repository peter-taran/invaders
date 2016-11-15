#include "stdafx.h"
#include "Viewport.h"
#include "Sprite.h"
#include "GameConstants.h"


Viewport::Viewport():
    _consoleSize(),
    _consoleCharCount(0),
    _showGame(false)
{}

Viewport::~Viewport()
{}

Viewport::Console::Console():
    handle(CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL,
        CONSOLE_TEXTMODE_BUFFER, NULL))
{}

Viewport::Console::~Console()
{
    CloseHandle(handle);
}

void Viewport::Console::resize(const DisplayCoord& sizes)
{
    // TODO: check Windows results for errors, here and everywhere

    SetConsoleScreenBufferSize(handle, sizes);

    SMALL_RECT rect;
    zeroVar(rect);
    rect.Right = sizes.x - 1;
    rect.Bottom = sizes.y - 1;
    SetConsoleWindowInfo(handle, TRUE, &rect);

    SetConsoleOutputCP(CP_UTF8);

    CONSOLE_CURSOR_INFO cci;
    zeroVar(cci);
    cci.bVisible = FALSE;
    cci.dwSize = 100;
    SetConsoleCursorInfo(handle, &cci);

    SetConsoleTitle(L"Морсеане отакуют!");
}

void Viewport::resize(const DisplayCoord& size)
{
    _consoleSize = size;
    _consoleCharCount = _consoleSize.x * _consoleSize.y;
    
    _console.resize(size);
    
    allocZeroedArray(_drawBuff, _consoleCharCount);
}

void Viewport::switchDisplay(Mode mode)
{
    switch(mode)
    {
    case Mode_game:
        _showGame = true;
        SetConsoleActiveScreenBuffer(_console.handle);
        break;
    case Mode_stdout:
        _showGame = false;
        SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
        break;
    default: unreachable("B8FE5A8D-FFE9-4D59-B4B9-FA6C4EF43F7F");
    }
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

    WriteConsoleOutput(_console.handle, _drawBuff.get(), _consoleSize,
        DisplayCoord(), &allRect);
}

CHAR_INFO* Viewport::_drawCharPtr(const int x, const int y)
{
    return &_drawBuff[x + y *_consoleSize.x];
}

void Viewport::draw(const DisplayCoord& at, const Sprite& sprite)
{
    // TODO: clipping

    const DisplayCoord spriteSize = sprite.size();
    
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
    // TODO: clipping

    for(int y = rect.tl.y; y < rect.br.y; ++y)
    {
        CHAR_INFO* putChar = _drawCharPtr(rect.tl.x, y);
        for(int x = rect.tl.x; x < rect.br.x; ++x, ++putChar)
            putChar->Attributes = attr;
    }
}

void Viewport::drawTextLine(const wstring& text, const DisplayCoord& startAt)
{
    // TODO: clipping

    CHAR_INFO* putChar = _drawCharPtr(startAt.x, startAt.y);
    foreach(wchar_t ch, text)
    {
        putChar->Char.UnicodeChar = ch;
        ++putChar;
    }
}
