#include <iostream>
#include <limits>

#include "Cpu65XX.hpp"

unsigned char testProgram[] =
{
    0xA9, 0x01,         // A = 1
    0xA2, 0x00,         // X = 0
    0xA0, 0xff,         // Y = 255 or -1
    0xE8,               // X++
    0xC8,               // Y++
    0x38,               // carry = 1
    0x4C, 0x06, 0x00    // JMP to 0x0006 (X++ above)
};

int main(int argc, char** argv)
{
    Cpu65XX cpu(testProgram, sizeof(testProgram));

    while (1) 
    {
        int c;
        cpu.execute();
        std::cout << cpu.state() << std::endl;
        std::cin.ignore(std::numeric_limits <std::streamsize> ::max(), '\n');
    }

    return 0;
}
