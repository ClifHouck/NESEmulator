#include "../iNESFile.hpp"
#include "../Cpu65XX.hpp"

#include <iostream>
#include <fstream>

int main(int argc, char ** argv) {

    iNESFile testRom("nestest.nes");
    unsigned char mappedData[64 * 1024];
    // std::cout << testRom.headerInfo();
    std::copy(testRom.m_fileData + 16, 
              testRom.m_fileData + 16 + testRom.PRGROMDataSize(), 
              mappedData + 0x8000);
    std::copy(testRom.m_fileData + 16, 
              testRom.m_fileData + 16 + testRom.PRGROMDataSize(), 
              mappedData + 0xC000);
    
    Cpu65XX cpu(mappedData, 64 * 1024);

    cpu.setPC(0xC000);

    while(1) {
        cpu.tick();
        if (cpu.PC() == 0x6CCE) {
            break;
        }
    }

    return 0;
}
