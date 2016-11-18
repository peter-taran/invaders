#pragma once


// Game is only one
//extern class Game* g_gameObject;

// Round "physical" (double) coordinate to display coordinate
inline int physToDisp(const double coord)
{
    return static_cast<int>(coord < 0 ? coord - 0.5 : coord + 0.5);
}

// Transform display coordinate to "physical"
// +~0.5 because we suppose display coordinate points to center of character cell
inline double dispToPhys(const int coord)
{
    // why not 0.5? because in this case physToDisp(dispToPhys(100)) == 101
    return static_cast<double>(coord) + (coord < 0 ? -0.4999999999 : +0.4999999999);
}

// Coordinates or sizes of something displayable
struct DisplayCoords: equality_comparable<DisplayCoords>
{
    int x;
    int y;

    DisplayCoords():
        x(0), y(0)
    {}

    DisplayCoords(int x, int y):
        x(x), y(y)
    {}

    DisplayCoords(double x, double y):
        x(physToDisp(x)), y(physToDisp(y))
    {}

    DisplayCoords(const COORD& from):
        x(from.X), y(from.Y)
    {}

    bool operator==(const DisplayCoords& cmpWith) const
    { return x == cmpWith.x && y == cmpWith.y; }

    COORD asCOORD() const
    {
        const COORD result = {static_cast<short>(x), static_cast<short>(y)};
        return result;
    }
    
    operator COORD() const
    { return asCOORD(); }
};

// Rectangle in displayable coordinates
struct DisplayRect
{
    DisplayCoords tl; // top-left point, included
    DisplayCoords br; // bottom-right point, excluded

    DisplayRect()
    {}

    DisplayRect(const DisplayCoords& topLeft, int width, int height):
        tl(topLeft), br(topLeft.x + width, topLeft.y + height)
    {}

    int width() const {
        return br.x - tl.x;
    }
    int height() const {
        return br.y - tl.y;
    }

    SMALL_RECT asRECT() const
    {
        const SMALL_RECT result = {
            static_cast<short>(tl.x), static_cast<short>(tl.y),
            static_cast<short>(br.x-1), static_cast<short>(br.y-1)
        };
        return result;
    }

    operator SMALL_RECT() const
    { return asRECT(); }
};

// starting from Vista you can declare your own colors for console (look
// SetConsoleScreenBufferInfoEx), but here is standard ones
enum DisplayColor {
    DisplayColor_black          = 0x0,
    DisplayColor_blue           = 0x1,
    DisplayColor_green          = 0x2,
    DisplayColor_aqua           = 0x3,
    DisplayColor_red            = 0x4,
    DisplayColor_purple         = 0x5,
    DisplayColor_yellow         = 0x6,
    DisplayColor_white          = 0x7,

    DisplayColor_gray           = 0x8 | DisplayColor_black,
    DisplayColor_lightBlue      = 0x8 | DisplayColor_blue,
    DisplayColor_lightGreen     = 0x8 | DisplayColor_green,
    DisplayColorlightAqua       = 0x8 | DisplayColor_aqua,
    DisplayColor_lightRed       = 0x8 | DisplayColor_red,
    DisplayColor_lightPurple    = 0x8 | DisplayColor_purple,
    DisplayColor_lightYellow    = 0x8 | DisplayColor_yellow,
    DisplayColor_brightWhite    = 0x8 | DisplayColor_white,
};

class CharAttr
{
public:
    CharAttr(): // white on black
        _attrib(DisplayColor_white | (DisplayColor_black << 4))
        //_attrib(DisplayColor_lightAqua | (DisplayColor_black << 4))
    {}

    CharAttr(DisplayColor textColor, DisplayColor bkColor):
        _attrib(textColor | (bkColor << 4))
    {}

    WORD asWord() const
    { return _attrib; }
    operator WORD() const
    { return _attrib; }

private:
    WORD _attrib;
};

// layouts display rectangles starting bottom to top
class DisplayAreaVertLayout
{
    DisplayRect _rect;

public:
    DisplayAreaVertLayout(const DisplayRect& totalArea);

    void gap(int gapY);

    DisplayRect nextField(int height);
    DisplayRect restArea(); // all the area still unallocated
};

// Some object, processing going of time
class TimeEater
{
protected:
    virtual void eatTime(const double from, const double to) = 0;

public:
    TimeEater(WillBeInitedLater);
    void laterInit(const double startMoment);

    TimeEater(const double startMoment);

    double eatenUpTo() const;

    void eatTimeUpTo(const double moment);

private:
    double _eatenUpTo;
};

// Interface to object should be painted
class Drawable
{
public:
    virtual void drawYourself(class Viewport& viewport) = 0;
};
