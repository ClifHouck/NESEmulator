#include "PPU.hpp"

PPU::
PPU(Cpu65XX::Memory& cpuMemory) :
    m_NMI           (false),
    m_clock         (0),
    m_control       (cpuMemory.byteAt(0x2000)),
    m_mask          (cpuMemory.byteAt(0x2001)),
    m_status        (cpuMemory.byteAt(0x2002)),
    m_oamAddress    (cpuMemory.byteAt(0x2003)),
    m_oamData       (cpuMemory.byteAt(0x2004)),
    m_oamDMA        (cpuMemory.byteAt(0x4014)),
    m_scroll        (cpuMemory.byteAt(0x2005)),
    m_address       (cpuMemory.byteAt(0x2006)),
    m_data          (cpuMemory.byteAt(0x2007)),
    m_bitmap        (),
    m_spriteRAM     ()
{
    std::fill(m_bitmap, m_bitmap + (width*height*3), 0.0);
    std::fill(m_spriteRAM, m_spriteRAM + spriteRamSize, 0x00);
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
    std::fill(m_memory, m_memory + PPU_MEM_SIZE, 0x00);
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
    if (m_mask & 0x10) {
        renderBackground();
    }
    if (m_mask & 0x20) {
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
