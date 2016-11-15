#include "stdafx.h"
#include "Sprite.h"


const CHAR_INFO Sprite::NO_CHARS;

Sprite::Sprite()
{}

Sprite::~Sprite()
{}

void Sprite::_initByStrList(const CharAttr& monoAttr, const wstring* strings,
    int stringCount)
{
    if( !strings )
        return;

    // calculate size
    for(int rowIndex = 0; rowIndex < stringCount; ++rowIndex)
    {
        const wstring& str = strings[rowIndex];
        if( str.empty() )
            continue;
        
        _size.x = (std::max<int>)(_size.x, str.length());
        ++_size.y;
    }

    const int charCount = _size.x * _size.y;
    if( charCount > 0 )
    {
        allocZeroedArray(_chars, charCount);

        // move characters
        CHAR_INFO* dest = _chars.get();
        for(int rowIndex = 0; rowIndex < stringCount; ++rowIndex)
        {
            const wstring& str = strings[rowIndex];
            if( str.empty() )
                continue;
            for(int colIndex = 0; colIndex < _size.x; ++colIndex)
            {
                dest->Attributes = monoAttr;
                if( colIndex < static_cast<int>(str.length()) && str[colIndex] != ' ' )
                    dest->Char.UnicodeChar = str[colIndex];
                else
                    dest->Char.UnicodeChar = L'\0';
                ++dest;
            }
        }
    }
}
