#ifndef PPU_PALETTE_H
#define PPU_PALETTE_H

#include "utility/DataTypes.hpp"

class Palette 
{
public:
    Palette(u8_byte value);

    class Color 
    {
    public:
        Color(u8_byte red, u8_byte green, u8_byte blue);

        u8_byte red()   const;
        u8_byte green() const;
        u8_byte blue()  const;

    private:
        u8_byte m_red;
        u8_byte m_green;
        u8_byte m_blue;
    };

    static const Color* getColor(u8_byte luminance, u8_byte chrominance);

    Color   color() const;
    u8_byte value() const;

    static const unsigned int   LUMINANCE_SHIFT     = 4; 
    static const u8_byte        LUMINANCE_MASK      = 0x30;
    static const u8_byte        CHROMINANCE_MASK    = 0x0F;
    static const u8_byte        MAX_CHROMINANCE     = 16;
    static const u8_byte        MAX_LUMINANCE       = 4;

private:
    static const Color colors[MAX_LUMINANCE][MAX_CHROMINANCE];
    u8_byte            m_value;
    const Color        *m_color;
};

#endif //PPU_PALETTE_H
