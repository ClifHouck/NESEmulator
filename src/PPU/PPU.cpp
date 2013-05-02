#include "PPU.hpp"

#include <algorithm>

PPU::
PPU(Cpu65XX::Memory &cpuMemory, 
    Clock &clock) :
    PoweredDevice(this),
    ClockedDevice(clockDivisor),
    m_isFirstWrite(true),
    m_NMI(false),
    m_clock(clock),
    // Register information derived from: 
    // http://wiki.nesdev.com/w/index.php/PPU_power_up_state
    m_control       (&cpuMemory.byteAt(CONTROL_ADDRESS)),
    m_mask          (&cpuMemory.byteAt(MASK_ADDRESS)), 
    m_status        (&cpuMemory.byteAt(STATUS_ADDRESS), m_isFirstWrite),
    m_oamAddress    (&cpuMemory.byteAt(OAM_ADDRESS_ADDRESS)),
    m_oamData       (&cpuMemory.byteAt(OAM_DATA_ADDRESS), m_spriteRAM, m_oamAddress),
    m_oamDMA        (&cpuMemory.byteAt(OAM_DMA_ADDRESS)),
    m_scroll        (&cpuMemory.byteAt(SCROLL_ADDRESS), m_isFirstWrite),
    m_address       (&cpuMemory.byteAt(SCROLL_ADDRESS_ADDRESS), m_isFirstWrite, m_control),
    m_data          (&cpuMemory.byteAt(SCROLL_DATA_ADDRESS), m_address, cpuMemory),
    m_bitmap        (),
    m_spriteRAM     ()
{
    std::fill(m_bitmap, m_bitmap + bitmapSize, 0.0);
    std::fill(m_spriteRAM, m_spriteRAM + spriteRamSize, 0x00);

    for (Register* reg : 
            { (Register*)&m_control, 
              (Register*)&m_mask,
              (Register*)&m_status,
              (Register*)&m_oamAddress,
              (Register*)&m_oamData,
              (Register*)&m_scroll,
              (Register*)&m_address,
              (Register*)&m_data }) {
        m_registers.push_back(reg);
    }
}

PPU::
~PPU()
{
}

void
PPU::
tick()
{
    //Y-coordinate basically.
    m_currentScanline = (m_clock.count() / ticksPerScanline) - 1;
    // Don't draw -1 scanline...
    if (m_currentScanline < 0) { return; }
    //X-coordinate basically.
    m_currentCycle    =  m_clock.count() % ticksPerScanline;
}

PPU::Memory::
Memory() 
{
    std::fill(m_memory, m_memory + memorySize, 0x00);
}
    
PPU::Memory::
Memory(u8_byte * toLoad, unsigned int size)
{
    std::copy(toLoad, toLoad + size, m_memory);
}

// FIXME: This function, or whatever replaces it must route 
// reads and writes through register classes. This will likely
// require a change in interface.
u8_byte& 
PPU::Memory::
byteAt(const u16_word& address)
{
    u16_word trueAddress = address & 0x3FFF;

    // 0x3000-0x3EFF is a mirror of 0x2000-0x2EFF
    if (trueAddress >= 0x3000 &&
        trueAddress <  0x3F00) {
        trueAddress -= 0x1000;
    }
    // 0x3F20-0x3FFF are mirrors of 0x3F00-0x3F1F
    else if (trueAddress >= 0x3F20) {
        trueAddress = (trueAddress % 0x20) + 0x3F00;
    }

    return m_memory[trueAddress];
}

// Renders current pixel determined by the clock.
void 
PPU::
render()
{
    if (m_mask.showBackground()) {
        renderBackground();
    }
    if (m_mask.showSprites()) {
        renderSprites();
    }
}

void 
PPU::
renderBackground()
{
    //TODO 
}

void 
PPU::
renderSprites()
{
    //TODO
}

const float* 
PPU::
displayBuffer() const
{
    return m_bitmap;
}

void 
PPU::
resetImpl()
{
    // TODO: Handle memory.
    std::for_each(m_registers.begin(), m_registers.end(), [] (Register* reg) {
            reg->reset();
    });
}

void 
PPU::
powerOnImpl()
{
    // TODO: Handle memory.
    std::for_each(m_registers.begin(), m_registers.end(), [] (Register* reg) {
            reg->powerOn();
    });
}

void 
PPU::
powerOffImpl()
{
    // TODO: Handle memory.
    std::for_each(m_registers.begin(), m_registers.end(), [] (Register* reg) {
            reg->powerOff();
    });
}

RegisterBlock::data_t
RegisterBlock::
getData(address_t address) 
{
}
