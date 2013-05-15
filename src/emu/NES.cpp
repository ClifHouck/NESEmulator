#include "NES.hpp"

NES::
NES() :
    PoweredDevice(this),
    m_clock (clockHertz),
    m_cpu (m_memory),
    m_ppu (m_memory, m_clock),
    m_memory (m_ppu.registerBlock())
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

NES::MainMemory::
MainMemory(Memory &ppuRegisters) :
    Memory(MAIN_MEMORY_SIZE),
    m_ppuRegisters (ppuRegisters)
{
}

NES::MainMemory::data_t
NES::MainMemory::
getData(address_t address) 
{
    if (address >= (WORK_RAM_END + 1) &&
        address <  PPU_REGISTERS_BEGIN) {
        return getData(address % 0x0800);
    }
    else if (address >= PPU_REGISTERS_BEGIN && 
             address <  APU_REGISTERS_BEGIN) {
        return m_ppuRegisters.read(address);
    }
    return Memory::getData(address);
}

void
NES::MainMemory::
setData(address_t address, data_t data) 
{
    if (address >= (WORK_RAM_END + 1) &&
        address <  PPU_REGISTERS_BEGIN) {
        Memory::setData(address, data);
    }
    else if (address >= PPU_REGISTERS_BEGIN &&
             address <  APU_REGISTERS_BEGIN) {
        m_ppuRegisters.write(address, data);
    }
    return Memory::setData(address, data);
}
