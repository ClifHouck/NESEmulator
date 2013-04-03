#include "IO/iNESFile.hpp"
#include "CPU/Cpu65XX.hpp"

#include <iostream>
#include <fstream>

int main(int argc, char ** argv) {

    iNESFile testRom("nestest.nes");
    unsigned char mappedData[64 * 1024];

    std::copy(testRom.m_fileData + 16, 
              testRom.m_fileData + 16 + testRom.PRGROMDataSize(), 
              mappedData + 0x8000);
    std::copy(testRom.m_fileData + 16, 
              testRom.m_fileData + 16 + testRom.PRGROMDataSize(), 
              mappedData + 0xC000);
    
    Cpu65XX cpu(mappedData, 64 * 1024);

    cpu.setPC(0xC000);

    bool failed = false;
    std::string reason;

    unsigned int lastDownCount = 0;

    // Run the test ROM.
    while(1) {
        cpu.tick();

        unsigned int currentDownCycles = cpu.downCycles();
        if (cpu.downCycles() > lastDownCount) {
            std::cout << cpu.lastInstructionDebugOut();
        }
        lastDownCount = currentDownCycles;

        if (cpu.PC() == 0xC66E &&
            cpu.cycles() == 26553) {
            reason = "Pass";
            break;
        }

        if (cpu.cycles() > 27000) {
            reason = "Program ran for too many cycles";
            failed = true;
            break;
        }
    }

    //TODO: Compare current output with nestest.log and report if we passed or
    // failed.

    return failed;
}
