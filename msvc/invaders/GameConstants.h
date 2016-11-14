#pragma once


// fixed FPS
static const double FPS = 30; // frame/sec
static const double FRAME_TIME = 1.0 / FPS; // sec

// size of game field
static const DisplayCoord GAME_FIELD_SIZE(140, 50);

// Gun options
static const double SPAAG_MAX_SPEED = GAME_FIELD_SIZE.x / 1.0; // coords/sec
static const double SPAAG_SPEEDUP_TIME = 0.4; // time to achieve max speed, sec

// Status line colors
static const DisplayColor STATUS_LINE_BK   = DisplayColor_purple;
static const DisplayColor STATUS_LINE_TEXT = DisplayColor_brightWhite;

//static const CharAttr STATUS_LINE_ATTR(DisplayColor_brightWhite, DisplayColor_blue);
