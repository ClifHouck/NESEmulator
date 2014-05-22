#include "PPU.hpp"

#include <algorithm>

PPU::
PPU(Memory *cpuMemory, 
    Clock &clock) :
    PoweredDevice(this),
    ClockedDevice(clockDivisor),
    m_isFirstWrite(true),
    m_NMI(false),
    m_clock(clock),
    // Register information derived from: 
    // http://wiki.nesdev.com/w/index.php/PPU_power_up_state
    m_control       (),
    m_mask          (), 
    m_status        (m_isFirstWrite),
    m_oamAddress    (),
    m_spriteRAM     (new BackedMemory(spriteStartAddress, spriteEndAddress)),
    m_oamData       (m_spriteRAM, m_oamAddress),
    m_oamDMA        (),
    m_scroll        (m_isFirstWrite),
    m_address       (m_isFirstWrite, m_control),
    m_data          (m_address, cpuMemory),
    m_bitmap        (),
    m_memory        (new BackedMemory(ppuStartAddress, ppuEndAddress)),
    m_registerBlock (*this)
{

    std::fill(m_bitmap, m_bitmap + bitmapSize, 0.0);

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
    delete m_spriteRAM;
    delete m_memory;
}

void
PPU::
setCartridgeMemory(Memory * ppuMemory)
{
    m_memory = ppuMemory;
}

void
PPU::
tick()
{
    // Y-coordinate basically.
    m_currentScanline = (m_clock.count() / ticksPerScanline) - 1;

    // Don't draw -1 scanline...
    // FIXME: Line below is incorrect.
    // if (m_currentScanline < 0) { return; }

    // X-coordinate basically.
    m_currentCycle    =  m_clock.count() % ticksPerScanline;
}

PPU::RegisterBlock&
PPU::
registerBlock()
{
    return m_registerBlock;
}

#if 0
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
#endif

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

Register*
PPU::RegisterBlock::
getRegister(RegisterBlock::address_t address)
{
    Register *reg = nullptr;
    switch (address) {
        case CONTROL_ADDRESS:
            reg = &m_ppu.m_control;
            break;
        case MASK_ADDRESS:
            reg = &m_ppu.m_mask;
            break;
        case STATUS_ADDRESS:
            reg = &m_ppu.m_status;
            break;
        case OAM_ADDRESS_ADDRESS:
            reg = &m_ppu.m_oamAddress;
            break;
        case OAM_DATA_ADDRESS:
            reg = &m_ppu.m_oamData;
            break;
        case OAM_DMA_ADDRESS:
            reg = &m_ppu.m_oamData;
            break;
        case SCROLL_ADDRESS:
            reg = &m_ppu.m_scroll;
            break;
        case SCROLL_ADDRESS_ADDRESS:
            reg = &m_ppu.m_address;
            break;
        case SCROLL_DATA_ADDRESS:
            reg = &m_ppu.m_data;
            break;
    }
    assert(reg != nullptr);
    return reg;
}

PPU::RegisterBlock::data_t
PPU::RegisterBlock::
getData(address_t address) 
{
    Register *reg = getRegister(address);
    return reg->read();
}

void
PPU::RegisterBlock::
setData(address_t address, data_t data)
{
    Register *reg = getRegister(address);
    reg->write(data);
}
