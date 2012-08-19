#ifndef PPU_H
#define PPU_H

#include "DataTypes.hpp"
#include "Cpu65XX.hpp"

class PPU
{
    public:
        PPU(Cpu65XX::Memory& cpuMemory);

        const static unsigned int width             = 256;
        const static unsigned int height            = 240;
        const static unsigned int ticksPerScanline  = 341;
        const static unsigned int memorySize        = 16 * 1024;
        const static unsigned int spriteRamSize     = 256;

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

        
    private:
        bool m_NMI;

        unsigned int m_clock;

        // PPU Control and Status Registers
        u8_byte& m_control;
        u8_byte& m_mask;
        u8_byte& m_status;

        // PPU Object Attribute Memory registers
        u8_byte& m_oamAddress;
        u8_byte& m_oamData;
        u8_byte& m_oamDMA;

        // PPU VRAM Access Registers
        u8_byte& m_scroll;
        u8_byte& m_address;
        u8_byte& m_data;

        // PPU Memory
        Memory  m_memory;
        u8_byte m_spriteRAM[spriteRamSize];

        // Rendering that we can display.
        float   m_bitmap[width * height * 3];
};

#endif
