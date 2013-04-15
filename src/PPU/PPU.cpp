#include "PPU.hpp"

#include <algorithm>

PPU::
PPU(Cpu65XX::Memory& cpuMemory) :
    PoweredDevice(this),
    m_NMI           (false),
    m_clock         (0),
    // Register information derived from: 
    // http://wiki.nesdev.com/w/index.php/PPU_power_up_state
    //              Backing                     On    Reset     RO Mask     Reset Mask 
    m_control       (&cpuMemory.byteAt(0x2000),  0x00, 0x00,       0x00,           0x00),
    m_mask          (&cpuMemory.byteAt(0x2001),  0x00, 0x00,       0x00,           0x00),
    m_status        (&cpuMemory.byteAt(0x2002),  0xA0, 0x00,       0x00,           0x80),
    m_oamAddress    (&cpuMemory.byteAt(0x2003),  0x00, 0x00,       0x00,           0xFF),
    // TODO: Add write-only behavior here.
    m_oamData       (&cpuMemory.byteAt(0x2004),  0x00, 0x00,       0x00,           0x00),
    m_oamDMA        (&cpuMemory.byteAt(0x4014),  0x00, 0x00,       0x00,           0x00),
    m_scroll        (&cpuMemory.byteAt(0x2005),  0x00, 0x00,       0x00,           0x00),
    m_address       (&cpuMemory.byteAt(0x2006),  0x00, 0x00,       0x00,           0xFF),
    m_data          (&cpuMemory.byteAt(0x2007),  0x00, 0x00,       0x00,           0x00),
    m_bitmap        (),
    m_spriteRAM     ()
{
    std::fill(m_bitmap, m_bitmap + (width*height*3), 0.0);
    std::fill(m_spriteRAM, m_spriteRAM + spriteRamSize, 0x00);

    for (Register* reg : 
            { &m_control, 
              &m_mask,
              &m_status,
              &m_oamAddress,
              &m_oamData,
              &m_oamDMA,
              &m_scroll,
              &m_address,
              &m_data }) {
        m_registers.push_back(reg);
    }
}

void
PPU::
tick()
{
    m_clock++;

    //Y-coordinate basically.
    m_currentScanline = (m_clock / ticksPerScanline) - 1;
    // Don't draw -1 scanline...
    if (m_currentScanline < 0) { return; }
    //X-coordinate basically.
    m_currentCycle    =  m_clock % ticksPerScanline;
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
    // TODO: Push masks behind defined constants
    if (m_mask.rawRead() & 0x10) {
        renderBackground();
    }
    if (m_mask.rawRead() & 0x20) {
        renderSprites();
    }
}

void 
PPU::
renderBackground()
{
}

void 
PPU::
renderSprites()
{
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
