#include "NES.hpp"

NES::
NES() :
    PoweredDevice(this),
    m_clock (clockHertz),
    m_cpu (),
    m_ppu (m_cpu.memory(), m_clock)
{
    m_clock.registerDevice(&m_cpu);
    m_clock.registerDevice(&m_ppu);
}

void 
NES::
tick() 
{
    m_clock.tick();
}

void
NES::
resetImpl()
{
    m_cpu.reset();
    m_ppu.reset();
}

void
NES::
powerOnImpl()
{
    m_cpu.powerOn();
    m_ppu.powerOn();
}

void
NES::
powerOffImpl()
{
    m_cpu.powerOff();
    m_ppu.powerOff();
}

MainMemory::data_t
MainMemory::
getData(address_t address) 
{
    if (address >= (WORK_RAM_END + 1) &&
        adresss <  PPU_REGISTERS_BEGIN) {
        return getData(address % 0x0800);
    }
    else if (address >= PPU_REGISTERS_BEGIN && 
             address <  APU_REGISTERS_BEGIN) {
        return m_ppuMemory.getData(address);
    }
    return Memory::getData(address);
}

void
MainMemory::
setData(address_t address, data_t data) 
{
    if (address >= (WORK_RAM_END + 1) &&
        address <  PPU_REGISTERS_BEGIN) {
        Memory::setData(address, data);
    }
    else if (address >= PPU_REGISTERS_BEGIN &&
             address <  APU_REGISTERS_BEGIN) {
        m_ppuMemory.setData(address, data);
    }
    return Memory::setData(address, data);
}
