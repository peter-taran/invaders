#include "stdafx.h"
#include "StatusLine.h"
#include "GameConstants.h"
#include "Viewport.h"


StatusLine::StatusLine(const DisplayCoord& viewportSize):
    _charCount(viewportSize.x),
    _bottomIndex(viewportSize.y - 3)
{}

StatusLine::~StatusLine()
{}

void StatusLine::drawYourself(Viewport& viewport)
{
    //static const wchar_t UPPER_CHAR = L'‗';
    //static const wchar_t UPPER_CHAR = L'▬';
    //static const wchar_t UPPER_CHAR = L'―';
    static const wchar_t UPPER_CHAR = L'▄';
    viewport.fillAttr(CharAttr(STATUS_LINE_BK, DisplayColor_black),
        DisplayRect(DisplayCoord(0, _bottomIndex), _charCount, 1));
    viewport.drawTextLine(
        wstring(_charCount, UPPER_CHAR),
        DisplayCoord(0, _bottomIndex+0));
    
    viewport.fillAttr(CharAttr(STATUS_LINE_TEXT, STATUS_LINE_BK),
        DisplayRect(DisplayCoord(0, _bottomIndex+1), _charCount, 2));

    static const wstring score1    = L"Humans : Invaders";
    static const wstring score2    = L"   420 : 108     ";

    viewport.drawTextLine(score1, DisplayCoord(1, _bottomIndex+1));
    viewport.drawTextLine(score2, DisplayCoord(1, _bottomIndex+2));
}
