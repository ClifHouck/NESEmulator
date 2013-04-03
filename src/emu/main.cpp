#include <iostream>
#include <limits>

#include "NesApp.hpp"

unsigned char testProgram[] =
{
    0xA9, 0x01, // A = 1
    0xA2, 0x00, // X = 0
    0xA0, 0xff, // Y = 255 or -1
    0xE8, // X++
    0xC8, // Y++
    0x38, // carry = 1
    0x4C, 0x06, 0x00 // JMP to 0x0006 (X++ above)
};

int main(int argc, char** argv)
{
    NESApp nes;

    return nes.onExecute();
}
