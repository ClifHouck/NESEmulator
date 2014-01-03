#ifndef NROM_MAPPER_H
#define NROM_MAPPER_H

#include "Mapper.hpp"

class NROMMapper : public Mapper
{
public:
    NROMMapper(iNESFile &file); 

    virtual Memory *cpuMemory(); 
    virtual Memory *ppuMemory();

    virtual const char* name() const { return "NROM"; }

    // CPU Banks.
    static const Memory::address_t PRG_RAM_BANK_BEGIN           = 0x6000;
    static const Memory::address_t PRG_RAM_BANK_END             = 0x7FFF;
    static const Memory::address_t PRG_ROM_BANK_BEGIN           = 0x8000;
    static const Memory::address_t PRG_ROM_BANK_2ND_HALF        = 0xC000;
    static const Memory::address_t PRG_ROM_BANK_END             = 0xFFFF;
    
    // PPU Banks.
    static const Memory::address_t VROM_BANK_BEGIN     = 0x0000;
    static const Memory::address_t VROM_BANK_END       = 0x1FFF;

    // Basic bank info.
    static const unsigned int PRG_BANK_SIZE  = 32 * 1024;
    static const unsigned int NUM_PRG_BANKS  = 1;
    static const unsigned int VROM_BANK_SIZE = 8 * 1024;
    static const unsigned int NUM_VROM_BANKS = 1;

private:
    iNESFile     m_rom;
    BackedMemory m_cpuMemory;
    BackedMemory m_ppuMemory;
};

#endif //NROM_MAPPER_H
