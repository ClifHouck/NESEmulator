#ifndef PPU_H
#define PPU_H

#include "utility/DataTypes.hpp"
#include "utility/Register.hpp"
#include "utility/PoweredDevice.hpp"
#include "CPU/Cpu65XX.hpp"

#include <vector>

class PPU : public PoweredDevice
{
public:
    PPU(Cpu65XX::Memory& cpuMemory);

    const static unsigned int width             = 256;
    const static unsigned int height            = 240;
    const static unsigned int ticksPerScanline  = 341;
    const static unsigned int memorySize        = 16 * 1024;
    const static unsigned int spriteRamSize     = 256;

    // FIXME: Remove this and make a general memory class.
    class Memory {
        public:
            Memory();
            Memory(u8_byte * toLoad, unsigned int size);

            u8_byte& byteAt(const u16_word& address);

        private:
            u8_byte m_memory[memorySize];
    };

    void tick();

    void signalNMI();

    void render();
    void renderBackground();
    void renderSprites();

    const float* displayBuffer() const;

protected:
    void resetImpl();
    void powerOnImpl();
    void powerOffImpl();
        
private:
    bool m_NMI;

    unsigned int m_clock;

    // PPU Control and Status Registers
    Register m_control;
    Register m_mask;
    Register m_status;

    // PPU Object Attribute Memory registers
    Register m_oamAddress;
    Register m_oamData;
    Register m_oamDMA;

    // PPU VRAM Access Registers
    Register m_scroll;
    Register m_address;
    Register m_data;

    std::vector<Register*> m_registers;

    // PPU Memory
    Memory  m_memory;
    u8_byte m_spriteRAM[spriteRamSize];

    // Rendering that we can display.
    float   m_bitmap[width * height * 3];

    unsigned int m_currentScanline;
    unsigned int m_currentCycle;
};

#endif
