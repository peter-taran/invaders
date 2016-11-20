#pragma once


// fixed FPS
static const double FPS = 33; // frame/sec
static const double FRAME_TIME = 1.0 / FPS; // sec

// how ofter we need to clean
static const int CLEAN_UP_EVERY_NTH_FRAME = static_cast<int>(FPS) * 5; // every N seconds

// Gun options
static const double GUN_MAX_SPEED = 150; // coords/sec
static const double GUN_SPEEDUP_TIME = 0.4; // time to achieve max speed, sec
static const double GUN_SHELL_SPEED = 24; // shell speed
static const double GUN_DIST_AFTER_FIRST = 3;
static const double GUN_DIST_IN_BIRST = 2;

// Status line colors
static const DisplayColor STATUS_LINE_BK   = DisplayColor_purple;
static const DisplayColor STATUS_LINE_TEXT = DisplayColor_brightWhite;
//static const CharAttr STATUS_LINE_ATTR(DisplayColor_brightWhite, DisplayColor_blue);

// How many humans we protecting
static const int HUMAN_NUMBER = 10;
static const int HUMAN_DIST_BETWEEN = 8;
static const int HUMAN_SPACE_LEFTRIGHT = 20;

// Echelongs for invider's ships
static const int ECHELONG_NEVER_USED_SPACE      = 15;
static const int ECHELONG_KAMIKAZE_AREA_HEIGHT  = 8;
static const int ECHELONG_LOW_COUNT             = 4;
static const int ECHELONG_LOW_HEIGHT            = 3;
static const int ECHELONG_HIGH_COUNT            = 1;
static const int ECHELONG_HIGH_HEIGHT           = 5;

// Invader ships
static const double SHIP_BASIC_FREQUENCY = 1; // one ship per SHIP_BASIC_FREQUENCY
static const double SHIP_REGUALAR_SPEED = GUN_MAX_SPEED / 6;

// Bombing
static const double BOMB_MAX_SPEED = 15;
static const double BOMB_SPEED_UP_TIME = 0.5;
