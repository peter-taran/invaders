#pragma once


// fixed FPS
static const double FPS = 30; // frame/sec
static const double FRAME_TIME = 1.0 / FPS; // sec

// how ofter we need to clean
static const int CLEAN_UP_EVERY_NTH_FRAME = static_cast<int>(FPS) * 5; // every N seconds

// Gun options
static const double GUN_MAX_SPEED = 150; // coords/sec
static const double GUN_SPEEDUP_TIME = 0.4; // time to achieve max speed, sec

// Status line colors
static const DisplayColor STATUS_LINE_BK   = DisplayColor_purple;
static const DisplayColor STATUS_LINE_TEXT = DisplayColor_brightWhite;
//static const CharAttr STATUS_LINE_ATTR(DisplayColor_brightWhite, DisplayColor_blue);

// How many humans we protecting
static const int HUMAN_NUMBER = 10;
static const int HUMAN_DIST_BETWEEN = 8;
static const int HUMAN_SPACE_LEFTRIGHT = 20;
