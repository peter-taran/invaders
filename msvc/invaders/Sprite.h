#pragma once


class Sprite: noncopyable
{
public:
    Sprite(); // empty sprite
    ~Sprite();

    template<int SIZE>
    Sprite(const array<wstring, SIZE>& strings, const CharAttr& monoAttr = CharAttr())
    { _initByStrList(monoAttr, strings.data(), strings.static_size); }

    DisplayCoords size() const
    { return _size; }

    int charCount() const
    { return _size.x * _size.y; }

    const CHAR_INFO& chars() const // size of of array = charCount()
    { return _chars ? *_chars.get() : NO_CHARS; }

private:
    DisplayCoords _size;
    scoped_array<CHAR_INFO> _chars;
    static const CHAR_INFO NO_CHARS;

    void _initByStrList(const CharAttr& monoAttr, const wstring* strings,
        int stringCount);
};
