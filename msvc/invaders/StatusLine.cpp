﻿#include "stdafx.h"
#include "StatusLine.h"
#include "GameConstants.h"
#include "Viewport.h"


StatusLine::StatusLine(const DisplayRect& area):
    _charCount(area.width()),
    _bottomIndex(area.tl.y)
{
    assert(area.height() == height());
}

StatusLine::~StatusLine()
{}

int StatusLine::height()
{
    return 3;
}

void StatusLine::drawYourself(Viewport& viewport)
{
    //static const wchar_t UPPER_CHAR = L'‗';
    //static const wchar_t UPPER_CHAR = L'▬';
    //static const wchar_t UPPER_CHAR = L'―';
    static const wchar_t UPPER_CHAR = L'▄';
    viewport.fillAttr(CharAttr(STATUS_LINE_BK, DisplayColor_black),
        DisplayRect(DisplayCoords(0, _bottomIndex), _charCount, 1));
    viewport.drawTextLine(
        wstring(_charCount, UPPER_CHAR),
        DisplayCoords(0, _bottomIndex+0));
    
    viewport.fillAttr(CharAttr(STATUS_LINE_TEXT, STATUS_LINE_BK),
        DisplayRect(DisplayCoords(0, _bottomIndex+1), _charCount, 2));

    static const wstring score1    = L"Humans : Invaders";
    static const wstring score2    = L"   420 : 108     ";

    viewport.drawTextLine(score1, DisplayCoords(1, _bottomIndex+1));
    viewport.drawTextLine(score2, DisplayCoords(1, _bottomIndex+2));
}
