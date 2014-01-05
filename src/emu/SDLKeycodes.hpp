#ifndef SDL_KEYCODE_TO_CHAR_H
#define SDL_KEYCODE_TO_CHAR_H

#include "SDL2/SDL.h"

#include <map>

typedef std::map<SDL_Keycode, unsigned short> KeycodeMap;
KeycodeMap SDLKeycodesToChar = 
{
    { SDLK_UNKNOWN, 0x00 },
    { SDLK_BACKSPACE, 0x08 },
    { SDLK_TAB, 0x09 },
    { SDLK_RETURN, 0x0D },
    { SDLK_ESCAPE, 0x1B },
    { SDLK_SPACE, 0x20 },
    { SDLK_EXCLAIM, 0x21 },
    { SDLK_QUOTEDBL, 0x22 },
    { SDLK_HASH, 0x23 },
    { SDLK_DOLLAR, 0x24 },
    { SDLK_PERCENT, 0x25 },
    { SDLK_AMPERSAND, 0x26 },
    { SDLK_QUOTE, 0x27 },
    { SDLK_LEFTPAREN, 0x28 },
    { SDLK_RIGHTPAREN, 0x29 },
    { SDLK_ASTERISK, 0x2A },
    { SDLK_PLUS, 0x2B },
    { SDLK_COMMA, 0x2C },
    { SDLK_MINUS, 0x2D },
    { SDLK_PERIOD, 0x2E },
    { SDLK_SLASH, 0x2F },
    { SDLK_0, 0x30 },
    { SDLK_1, 0x31 },
    { SDLK_2, 0x32 },
    { SDLK_3, 0x33 },
    { SDLK_4, 0x34 },
    { SDLK_5, 0x35 },
    { SDLK_6, 0x36 },
    { SDLK_7, 0x37 },
    { SDLK_8, 0x38 },
    { SDLK_9, 0x39 },
    { SDLK_COLON, 0x3A },
    { SDLK_SEMICOLON, 0x3B },
    { SDLK_LESS, 0x3C },
    { SDLK_EQUALS, 0x3D },
    { SDLK_GREATER, 0x3E },
    { SDLK_QUESTION, 0x3F },
    { SDLK_AT, 0x40 },
    { SDLK_LEFTBRACKET, 0x5B },
    { SDLK_BACKSLASH, 0x5C },
    { SDLK_RIGHTBRACKET, 0x5D },
    { SDLK_CARET, 0x5E },
    { SDLK_UNDERSCORE, 0x5F },
    { SDLK_BACKQUOTE, 0x60 },
    { SDLK_a, 0x61 },
    { SDLK_b, 0x62 },
    { SDLK_c, 0x63 },
    { SDLK_d, 0x64 },
    { SDLK_e, 0x65 },
    { SDLK_f, 0x66 },
    { SDLK_g, 0x67 },
    { SDLK_h, 0x68 },
    { SDLK_i, 0x69 },
    { SDLK_j, 0x6A },
    { SDLK_k, 0x6B },
    { SDLK_l, 0x6C },
    { SDLK_m, 0x6D },
    { SDLK_n, 0x6E },
    { SDLK_o, 0x6F },
    { SDLK_p, 0x70 },
    { SDLK_q, 0x71 },
    { SDLK_r, 0x72 },
    { SDLK_s, 0x73 },
    { SDLK_t, 0x74 },
    { SDLK_u, 0x75 },
    { SDLK_v, 0x76 },
    { SDLK_w, 0x77 },
    { SDLK_x, 0x78 },
    { SDLK_y, 0x79 },
    { SDLK_z, 0x7A },
    { SDLK_DELETE, 0xB1 },
    { SDLK_CAPSLOCK, 0x40000039},
    { SDLK_F1, 0x4000003A},
    { SDLK_F2, 0x4000003B},
    { SDLK_F3, 0x4000003C},
    { SDLK_F4, 0x4000003D},
    { SDLK_F5, 0x4000003E},
    { SDLK_F6, 0x4000003F},
    { SDLK_F7, 0x40000040},
    { SDLK_F8, 0x40000041},
    { SDLK_F9, 0x40000042},
    { SDLK_F10, 0x40000043},
    { SDLK_F11, 0x40000044},
    { SDLK_F12, 0x40000045},
    { SDLK_PRINTSCREEN, 0x40000046},
    { SDLK_SCROLLLOCK, 0x40000047},
    { SDLK_PAUSE, 0x40000048},
    { SDLK_INSERT, 0x40000049},
    { SDLK_HOME, 0x4000004A},
    { SDLK_PAGEUP, 0x4000004B},
    { SDLK_END, 0x4000004D},
    { SDLK_PAGEDOWN, 0x4000004E},
    { SDLK_RIGHT, 0x4000004F},
    { SDLK_LEFT, 0x40000050},
    { SDLK_DOWN, 0x40000051},
    { SDLK_UP, 0x40000052},
    { SDLK_NUMLOCKCLEAR, 0x40000053},
    { SDLK_KP_DIVIDE, 0x40000054},
    { SDLK_KP_MULTIPLY, 0x40000055},
    { SDLK_KP_MINUS, 0x40000056},
    { SDLK_KP_PLUS, 0x40000057},
    { SDLK_KP_ENTER, 0x40000058},
    { SDLK_KP_1, 0x40000059},
    { SDLK_KP_2, 0x4000005A},
    { SDLK_KP_3, 0x4000005B},
    { SDLK_KP_4, 0x4000005C},
    { SDLK_KP_5, 0x4000005D},
    { SDLK_KP_6, 0x4000005E},
    { SDLK_KP_7, 0x4000005F},
    { SDLK_KP_8, 0x40000060},
    { SDLK_KP_9, 0x40000061},
    { SDLK_KP_0, 0x40000062},
    { SDLK_KP_PERIOD, 0x40000063},
    { SDLK_APPLICATION, 0x40000065},
    { SDLK_POWER, 0x40000066},
    { SDLK_KP_EQUALS, 0x40000067},
    { SDLK_F13, 0x40000068},
    { SDLK_F14, 0x40000069},
    { SDLK_F15, 0x4000006A},
    { SDLK_F16, 0x4000006B},
    { SDLK_F17, 0x4000006C},
    { SDLK_F18, 0x4000006D},
    { SDLK_F19, 0x4000006E},
    { SDLK_F20, 0x4000006F},
    { SDLK_F21, 0x40000070},
    { SDLK_F22, 0x40000071},
    { SDLK_F23, 0x40000072},
    { SDLK_F24, 0x40000073},
    { SDLK_EXECUTE, 0x40000074},
    { SDLK_HELP, 0x40000075},
    { SDLK_MENU, 0x40000076},
    { SDLK_SELECT, 0x40000077},
    { SDLK_STOP, 0x40000078},
    { SDLK_AGAIN, 0x40000079},
    { SDLK_UNDO, 0x4000007A},
    { SDLK_CUT, 0x4000007B},
    { SDLK_COPY, 0x4000007C},
    { SDLK_PASTE, 0x4000007D},
    { SDLK_FIND, 0x4000007E},
    { SDLK_MUTE, 0x4000007F},
    { SDLK_VOLUMEUP, 0x40000080},
    { SDLK_VOLUMEDOWN, 0x40000081},
    { SDLK_KP_COMMA, 0x40000085},
    { SDLK_KP_EQUALSAS400, 0x40000086},
    { SDLK_ALTERASE, 0x40000099},
    { SDLK_SYSREQ, 0x4000009A},
    { SDLK_CANCEL, 0x4000009B},
    { SDLK_CLEAR, 0x4000009C},
    { SDLK_PRIOR, 0x4000009D},
    { SDLK_RETURN2, 0x4000009E},
    { SDLK_SEPARATOR, 0x4000009F},
    { SDLK_OUT, 0x400000A0},
    { SDLK_OPER, 0x400000A1},
    { SDLK_CLEARAGAIN, 0x400000A2},
    { SDLK_CRSEL, 0x400000A3},
    { SDLK_EXSEL, 0x400000A4},
    { SDLK_KP_00, 0x400000B0},
    { SDLK_KP_000, 0x400000B1},
    { SDLK_THOUSANDSSEPARATOR, 0x400000B2},
    { SDLK_DECIMALSEPARATOR, 0x400000B3},
    { SDLK_CURRENCYUNIT, 0x400000B4},
    { SDLK_CURRENCYSUBUNIT, 0x400000B5},
    { SDLK_KP_LEFTPAREN, 0x400000B6},
    { SDLK_KP_RIGHTPAREN, 0x400000B7},
    { SDLK_KP_LEFTBRACE, 0x400000B8},
    { SDLK_KP_RIGHTBRACE, 0x400000B9},
    { SDLK_KP_TAB, 0x400000BA},
    { SDLK_KP_BACKSPACE, 0x400000BB},
    { SDLK_KP_A, 0x400000BC},
    { SDLK_KP_B, 0x400000BD},
    { SDLK_KP_C, 0x400000BE},
    { SDLK_KP_D, 0x400000BF},
    { SDLK_KP_E, 0x400000C0},
    { SDLK_KP_F, 0x400000C1},
    { SDLK_KP_XOR, 0x400000C2},
    { SDLK_KP_POWER, 0x400000C3},
    { SDLK_KP_PERCENT, 0x400000C4},
    { SDLK_KP_LESS, 0x400000C5},
    { SDLK_KP_GREATER, 0x400000C6},
    { SDLK_KP_AMPERSAND, 0x400000C7},
    { SDLK_KP_DBLAMPERSAND, 0x400000C8},
    { SDLK_KP_VERTICALBAR, 0x400000C9},
    { SDLK_KP_DBLVERTICALBAR, 0x400000CA},
    { SDLK_KP_COLON, 0x400000CB},
    { SDLK_KP_HASH, 0x400000CC},
    { SDLK_KP_SPACE, 0x400000CD},
    { SDLK_KP_AT, 0x400000CE},
    { SDLK_KP_EXCLAM, 0x400000CF},
    { SDLK_KP_MEMSTORE, 0x400000D0},
    { SDLK_KP_MEMRECALL, 0x400000D1},
    { SDLK_KP_MEMCLEAR, 0x400000D2},
    { SDLK_KP_MEMADD, 0x400000D3},
    { SDLK_KP_MEMSUBTRACT, 0x400000D4},
    { SDLK_KP_MEMMULTIPLY, 0x400000D5},
    { SDLK_KP_MEMDIVIDE, 0x400000D6},
    { SDLK_KP_PLUSMINUS, 0x400000D7},
    { SDLK_KP_CLEAR, 0x400000D8},
    { SDLK_KP_CLEARENTRY, 0x400000D9},
    { SDLK_KP_BINARY, 0x400000DA},
    { SDLK_KP_OCTAL, 0x400000DB},
    { SDLK_KP_DECIMAL, 0x400000DC},
    { SDLK_KP_HEXADECIMAL, 0x400000DD},
    { SDLK_LCTRL, 0x400000E0},
    { SDLK_LSHIFT, 0x400000E1},
    { SDLK_LALT, 0x400000E2},
    { SDLK_LGUI, 0x400000E3},
    { SDLK_RCTRL, 0x400000E4},
    { SDLK_RSHIFT, 0x400000E5},
    { SDLK_RALT, 0x400000E6},
    { SDLK_RGUI, 0x400000E7},
    { SDLK_MODE, 0x40000101},
    { SDLK_AUDIONEXT, 0x40000102},
    { SDLK_AUDIOPREV, 0x40000103},
    { SDLK_AUDIOSTOP, 0x40000104},
    { SDLK_AUDIOPLAY, 0x40000105},
    { SDLK_AUDIOMUTE, 0x40000106},
    { SDLK_MEDIASELECT, 0x40000107},
    { SDLK_WWW, 0x40000108},
    { SDLK_MAIL, 0x40000109},
    { SDLK_CALCULATOR, 0x4000010A},
    { SDLK_COMPUTER, 0x4000010B},
    { SDLK_AC_SEARCH, 0x4000010C},
    { SDLK_AC_HOME, 0x4000010D},
    { SDLK_AC_BACK, 0x4000010E},
    { SDLK_AC_FORWARD, 0x4000010F},
    { SDLK_AC_STOP, 0x40000110},
    { SDLK_AC_REFRESH, 0x40000111},
    { SDLK_AC_BOOKMARKS, 0x40000112},
    { SDLK_BRIGHTNESSDOWN, 0x40000113},
    { SDLK_BRIGHTNESSUP, 0x40000114},
    { SDLK_DISPLAYSWITCH, 0x40000115}
};

#endif //SDL_KEYCODE_TO_CHAR_H
