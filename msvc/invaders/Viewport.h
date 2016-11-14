#pragma once


class Viewport: noncopyable
{
public:
    Viewport();
    ~Viewport();

    void resize(const DisplayCoord& size);

    enum Mode {
        Mode_game,
        Mode_stdout,
    };
    void switchDisplay(Mode mode);
    
    void eraseDrawFrame();
    void switchFrame();

    void draw(const DisplayCoord& at, const class Sprite& sprite);

    void fillAttr(const CharAttr& attr, const DisplayRect& rect);

    void drawTextLine(const wstring& text, const DisplayCoord& startAt);

private:
    DisplayCoord _consoleSize;
    DWORD _consoleCharCount; // = _consoleSize.x * _consoleSize.y

    struct Console
    {
        HANDLE handle;

        Console();
        ~Console();

        void resize(const DisplayCoord& sizes);
    };
    bool _showGame;
    Console _console;

    scoped_array<CHAR_INFO> _drawBuff;
    CHAR_INFO* _drawCharPtr(const int x, const int y);
};
