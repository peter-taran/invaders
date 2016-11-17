#pragma once


class Viewport: noncopyable
{
public:
    Viewport();
    ~Viewport();

    void gameMode(const DisplayCoords& size);
    void stdMode(); // на консоли stdin/stdout
    
    void eraseDrawFrame();
    void switchFrame();

    void draw(const DisplayCoords& at, const class Sprite& sprite);

    void fillAttr(const CharAttr& attr, const DisplayRect& rect);

    void drawTextLine(const wstring& text, const DisplayCoords& startAt);

private:
    DisplayCoords _consoleSize;
    DWORD _consoleCharCount; // = _consoleSize.x * _consoleSize.y

    struct Console
    {
        HANDLE handle;

        Console(const DisplayCoords& size);
        ~Console();

        void resize(const DisplayCoords& sizes);
    };
    scoped_ptr<Console> _console;

    scoped_array<CHAR_INFO> _drawBuff;
    CHAR_INFO* _drawCharPtr(const int x, const int y);
};
