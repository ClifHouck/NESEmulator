#include "NROMMapper.hpp"

NROMMapper::
NROMMapper(iNESFile &file) :
    m_rom (file),
    m_cpuMemory(file.PRGROMDataSize(), file.prgRomPage(0)),
    m_ppuMemory(file.CHRROMDataSize(), file.vromPage(0))
{
    // FIXME: What if the data size in the file doesn't fill the needed space?
    m_cpuMemory.setAddressRange(PRG_ROM_BANK_BEGIN, PRG_ROM_BANK_BEGIN + file.PRGROMDataSize());
    // FIXME: What if the data size in the file doesn't fill the needed space?
    m_ppuMemory.setAddressRange(VROM_BANK_BEGIN, VROM_BANK_BEGIN + file.CHRROMDataSize());
}

Memory*
NROMMapper::
cpuMemory() 
{
    return &m_cpuMemory;
}

Memory*
NROMMapper::
ppuMemory() 
{
    return &m_ppuMemory;
}
