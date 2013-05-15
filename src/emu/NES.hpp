#ifndef NES_H
#define NES_H

#include "utility/DataTypes.hpp"
#include "utility/PoweredDevice.hpp"
#include "utility/Clock.hpp"
#include "utility/Memory.hpp"
#include "CPU/Cpu65XX.hpp"
#include "PPU/PPU.hpp"

class NES : public PoweredDevice
{
public:
    NES();

    void tick();

    static const unsigned int clockHertz = 21477270;

    class MainMemory : public Memory 
    {
    public:
        MainMemory(Memory &ppuRegisters);

        static const Memory::size_t MAIN_MEMORY_SIZE    = 2 * 1024;

        static const address_t WORK_RAM_BEGIN           = 0x0000;
        static const address_t WORK_RAM_END             = 0x07FF;
        static const address_t WORK_RAM_SIZE            = WORK_RAM_END - WORK_RAM_BEGIN + 1;
        static const address_t WORK_RAM_MIRROR_END      = 0x1FFF;
        static const address_t PPU_REGISTERS_BEGIN      = 0x2000;
        static const address_t PPU_REGISTERS_END        = 0x2007;
        static const address_t PPU_REGISTERS_SIZE       = PPU_REGISTERS_END - PPU_REGISTERS_BEGIN + 1;
        static const address_t PPU_MIRROR_END           = 0x3FFF;
        static const address_t APU_REGISTERS_BEGIN      = 0x4000;
        static const address_t APU_REGISTERS_END        = 0x4017;
        static const address_t APU_REGISTERS_SIZE       = APU_REGISTERS_END - APU_REGISTERS_BEGIN + 1;
        static const address_t CARTRIDGE_EXPO_BEGIN     = 0x4018;
        static const address_t CARTRIDGE_EXPO_END       = 0x5FFF;
        static const address_t CARTRIDGE_SRAM_BEGIN     = 0x6000;
        static const address_t CARTRIDGE_PRGROM_BEGIN   = 0x8000;
        static const address_t CARTRIDGE_PRGROM_END     = 0xFFFF;

    protected:
        virtual data_t getData(address_t address);
        virtual void   setData(address_t address, data_t data);

    private:
        Memory &m_ppuRegisters;
        //Memory &m_apuMemory;
    };

protected:
    virtual void resetImpl();
    virtual void powerOnImpl();
    virtual void powerOffImpl();

private:
    MainMemory m_memory;
    Cpu65XX m_cpu;
    PPU     m_ppu;
    Clock   m_clock;
};

#endif //NES_H
