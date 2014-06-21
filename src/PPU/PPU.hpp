#ifndef PPU_H
#define PPU_H

#include "utility/DataTypes.hpp"
#include "utility/Register.hpp"
#include "utility/PoweredDevice.hpp"
#include "utility/Clock.hpp"
#include "utility/Memory.hpp"
#include "CPU/Cpu65XX.hpp"

#include <vector>
#include <cassert>

class PPU : public PoweredDevice, public ClockedDevice
{
public:
    PPU(Memory *cpuMemory,
        Clock& clock);
    ~PPU();

    const static unsigned int width;             
    const static unsigned int height;            
    const static unsigned int ticksPerScanline;  
    const static unsigned int memorySize;        
    const static unsigned int spriteRamSize;     
    const static unsigned int bitmapSize;        
    const static unsigned int spriteSize;        
    const static unsigned int tileSize;          
    const static unsigned int clockDivisor;      

    const static Memory::address_t ppuStartAddress;  
    const static Memory::address_t ppuEndAddress;    

    const static Memory::address_t spriteStartAddress; 
    const static Memory::address_t spriteEndAddress;   

    const static u16_word CONTROL_ADDRESS;           
    const static u16_word MASK_ADDRESS;              
    const static u16_word STATUS_ADDRESS;            
    const static u16_word OAM_ADDRESS_ADDRESS;       
    const static u16_word OAM_DATA_ADDRESS;          
    const static u16_word OAM_DMA_ADDRESS;           
    const static u16_word SCROLL_ADDRESS;            
    const static u16_word SCROLL_ADDRESS_ADDRESS;    
    const static u16_word SCROLL_DATA_ADDRESS;       

    class RegisterBlock : public Memory 
    {
    public:
        static const address_t  baseAddress = 0x2000;
        static const address_t  lastAddress = 0x2008;

        RegisterBlock(PPU &ppu) : 
            Memory(baseAddress, lastAddress),
            m_ppu (ppu)
        {}

        Memory* clone() { return new RegisterBlock(*this); }

    protected:
        virtual data_t getData(address_t address);
        virtual void   setData(address_t address, data_t data);

        Register* getRegister(address_t address);

    private:
        PPU &m_ppu;
    };

    RegisterBlock& registerBlock();
    const RegisterBlock& registerBlock() const;

    void setCartridgeMemory(Memory *ppuMemory);

    virtual void tick();

    void signalNMI();

    void render();
    void renderBackground();
    void renderSprites();

    const float* displayBuffer() const;

protected:
    void resetImpl();
    void powerOnImpl();
    void powerOffImpl();
        
private:

    class PPUController : public WriteOnlyRegister
    {
    public:
        PPUController() :
            WriteOnlyRegister(StateData("ppu_controller", 0x00, 0x00, 0x00, 0x00))
        {}
        ~PPUController() {}

        static const u8_byte BASE_NAMETABLE_ADDRESS_MASK            = 0x03;
        static const u8_byte VRAM_ADDRESS_INCREMENT_MASK            = 0x04;
        static const u8_byte SPRITE_PATTERN_TABLE_ADDRESS_MASK      = 0x08;
        static const u8_byte BACKGROUND_PATTERN_TABLE_ADDRESS_MASK  = 0x10;
        static const u8_byte SPRITE_SIZE_MASK                       = 0x20;
        static const u8_byte MASTER_SLAVE_SELECT_MASK               = 0x40;
        static const u8_byte GENERATE_NMI_MASK                      = 0x80;

        u16_word baseNameTableAddress() const {
            return 0x2000 + ((rawRead() & BASE_NAMETABLE_ADDRESS_MASK) != 0) * 0x400;
        }
        bool vramAddressIncrement() const { return rawRead() & VRAM_ADDRESS_INCREMENT_MASK; }

        int addressIncrementAmount() const { 
            if (vramAddressIncrement()) {
                return 32;
            }
            return 1;
        }

        u16_word spritePatternTableAddress() const { 
            return ((rawRead() & SPRITE_PATTERN_TABLE_ADDRESS_MASK) != 0) * 0x1000; 
        }
        u16_word backgroundPatternTableAddress() const {
            return ((rawRead() & BACKGROUND_PATTERN_TABLE_ADDRESS_MASK) != 0) * 0x1000;
        }
        std::pair<unsigned int, unsigned int> spriteSize() const {
            return std::pair<unsigned int, unsigned int>(8, 8 + (8 * (rawRead() & SPRITE_SIZE_MASK)));
        }
        bool masterSlaveSelect() const { return rawRead() & MASTER_SLAVE_SELECT_MASK; }
        bool generateNMI() const { return rawRead() & GENERATE_NMI_MASK; }
    };

    class PPUMask : public Register 
    {
    public:
        PPUMask() :
            Register(StateData("ppu_mask", 0x00, 0x00, 0x00, 0x00))
        {}
        ~PPUMask() {}

        static const u8_byte GRAYSCALE_MASK                     = 0x01;
        static const u8_byte SHOW_LEFTMOST_BACKGROUND_MASK      = 0x02;
        static const u8_byte SHOW_LEFTMOST_SPRITES_MASK         = 0x04;
        static const u8_byte SHOW_BACKGROUND_MASK               = 0x08;
        static const u8_byte SHOW_SPRITES_MASK                  = 0x10;
        static const u8_byte INTENSIFY_REDS_MASK                = 0x20;
        static const u8_byte INTENSIFY_GREENS_MASK              = 0x40;
        static const u8_byte INTENSIFY_BLUES_MASK               = 0x80;

        bool grayscale() const { return rawRead() & GRAYSCALE_MASK; }
        bool showLeftmostBackground() const { return rawRead() & SHOW_LEFTMOST_BACKGROUND_MASK; }
        bool showLeftmostSprites() const { return rawRead() & SHOW_LEFTMOST_SPRITES_MASK; }
        bool showBackground() const { return rawRead() & SHOW_BACKGROUND_MASK; }
        bool showSprites() const { return rawRead() & SHOW_SPRITES_MASK; }
        bool intensifyReds() const { return rawRead() & INTENSIFY_REDS_MASK; }
        bool intensifyGreens() const { return rawRead() & INTENSIFY_GREENS_MASK; }
        bool intensifyBlues() const { return rawRead() & INTENSIFY_BLUES_MASK; }
    };

    class PPUStatus : public Register {
    public:
        PPUStatus(bool &isFirstWrite) :
            Register(StateData("ppu_status", 0xA0, 0x00, 0x00, 0x80)),
            m_isFirstWrite (isFirstWrite)
        {}
        ~PPUStatus() {}

        static const u8_byte LEAST_SIGNIFICANT_BITS_MASK    = 0x1F;
        static const u8_byte SPRITE_OVERFLOW_MASK           = 0x20;
        static const u8_byte SPRITE_0_HIT_MASK              = 0x40;
        static const u8_byte VERTICAL_BLANK_STARTED_MASK    = 0x80;

        virtual u8_byte read() {
            rawWrite(rawRead() & ~VERTICAL_BLANK_STARTED_MASK);
            m_isFirstWrite = true;
            return Register::read();
        }

        bool spriteOverflow() { return rawRead() & SPRITE_OVERFLOW_MASK; }
        bool sprite0Hit() { return rawRead() & SPRITE_0_HIT_MASK; }
        bool verticalBlank() { return rawRead() & VERTICAL_BLANK_STARTED_MASK; }

    private:
        bool &m_isFirstWrite;
    };

    class OAMAddress : public WriteOnlyRegister
    {
    public:
        OAMAddress() :
            WriteOnlyRegister(StateData("oam_address", 0x00, 0x00, 0x00, 0xFF))
        {}
        ~OAMAddress() {}

        void increment() { rawWrite(rawRead() + 1); }

        u8_byte address() const { return rawRead(); }
    };

    class OAMData : public Register
    {
    public:
        OAMData(Memory *&spriteRAM,
                OAMAddress &address) :
            Register(StateData("oam_data", 0x00, 0x00, 0x00, 0x00)),
            m_spriteRAM (spriteRAM),
            m_address(address)
        {}

        virtual u8_byte read() {
            rawWrite(m_spriteRAM->read(m_address.address()));
            return Register::read();
        }

        virtual void write(u8_byte data, u8_byte mask = 0xFF) {
            Register::write(data, mask);
            m_spriteRAM->write(m_address.address(), rawRead());
            m_address.increment();
        }

    private:
        Memory     *&m_spriteRAM;
        OAMAddress &m_address;
    };

    class OAMDMA : public WriteOnlyRegister 
    {
    public:
        OAMDMA() :
            WriteOnlyRegister(StateData("oam_dma", 0x00, 0x00, 0x00, 0x00))
        {}
        ~OAMDMA() {}

        u16_word address() const { return rawRead() * 0x100; }
    };

    class VRAMScroll : public WriteOnlyRegister
    {
    public:
        VRAMScroll(bool &isFirstWrite) :
            WriteOnlyRegister(StateData("vram_scroll", 0x00, 0x00, 0x00, 0xFF)),
            m_isFirstWrite (isFirstWrite)
        {}
        ~VRAMScroll() {}

        virtual void write(u8_byte data, u8_byte mask = 0xFF) {
            WriteOnlyRegister::write(data, mask);
            if (m_isFirstWrite) {
                m_horizontalScrollOrigin = rawRead();
            }
            else {
                m_verticalScrollOrigin = rawRead();
            }
            m_isFirstWrite = !m_isFirstWrite;
        }

        u8_byte horizontalScrollOrigin() const {
            return m_horizontalScrollOrigin;
        }

        u8_byte verticalScrollOrigin() const {
            return m_verticalScrollOrigin;
        }

    private:
        u8_byte m_horizontalScrollOrigin;
        u8_byte m_verticalScrollOrigin;
        bool   &m_isFirstWrite;
    };

    // (v) register
    class VRAMAddress : public WriteOnlyRegister
    {
    public:
        // Breakdown of register bits taken from:
        // http://wiki.nesdev.com/w/index.php/The_skinny_on_NES_scrolling
        static const u16_word COARSE_X_SCROLL_MASK  = 0x001F;
        static const u16_word COARSE_Y_SCROLL_MASK  = 0x03E0;
        static const u16_word NAMETABLE_SELECT_MASK = 0x0C00;
        static const u16_word FINE_Y_SCROLL_MASK    = 0x7000;

        VRAMAddress(bool &isFirstWrite,
                    PPUController &ppuController) :
            WriteOnlyRegister(StateData("vram_address", 0x00, 0x00, 0x00, 0xFF)),
            m_isFirstWrite (isFirstWrite),
            m_ppuController (ppuController),
            m_highByte (0x00),
            m_lowByte (0x00)
        {}
        ~VRAMAddress() {}

        u8_byte coarseXScroll() const {
            return address() & COARSE_X_SCROLL_MASK;
        }

        u8_byte coarseYScroll() const {
            return (address() & COARSE_Y_SCROLL_MASK) >> 5; 
        }

        u8_byte nametableSelect() const {
            return (address() & NAMETABLE_SELECT_MASK) >> 10;
        }

        u8_byte fineYScroll() const {
            return (address() & FINE_Y_SCROLL_MASK) >> 12;
        }

        u16_word address() const {
            return m_address;
        }

        virtual void write(u8_byte data, u8_byte mask = 0xFF) {
            WriteOnlyRegister::write(data, mask);
            if (m_isFirstWrite) {
                m_highByte = rawRead();
                m_address  = (m_highByte * 0x100) | (m_address & 0x00FF);
            }
            else {
                m_lowByte = rawRead();
                m_address = (m_address & 0xFF00) | m_lowByte;
            }
            m_isFirstWrite = !m_isFirstWrite;
        }

        void increment() {
            rawWrite(rawRead() + m_ppuController.addressIncrementAmount());
        }

    private:
        PPUController  &m_ppuController;
        bool           &m_isFirstWrite;
        u8_byte         m_highByte;
        u8_byte         m_lowByte;
        u16_word        m_address;
    };

    class VRAMData : public Register 
    {
    public:
        VRAMData(VRAMAddress &vramAddress,
                 Memory *cpuMemory) :
            Register(StateData("vram_data", 0x00, 0x00, 0x00, 0x00)),
            m_vramAddress (vramAddress),
            m_cpuMemory (cpuMemory)
        {}
        ~VRAMData() {}

        virtual u8_byte read() {
            // Load the data from CPU memory into this register.
            Register::rawWrite(m_cpuMemory->read(m_vramAddress.address()));
            m_vramAddress.increment();
            return Register::read();
        }
        
        virtual void write(u8_byte data, u8_byte mask = 0xFF) {
            u8_byte vramData = m_cpuMemory->read(m_vramAddress.address());
            // Update the contents of this register.
            rawWrite(vramData);
            Register::write(data, mask);
            // Write back to the CPU's memory.
            m_cpuMemory->write(m_vramAddress.address(), rawRead());
            m_vramAddress.increment();
        }

    private:
        VRAMAddress         &m_vramAddress;
        Memory              *m_cpuMemory; 
    };

    class Tile
    {
    public:
        Tile(u8_byte *backing) :
            m_backing (backing)
        {}
        ~Tile() {}

        static const unsigned int sideLength = 8;
        static const unsigned int byteSize   = 16;

        u8_byte color(unsigned int x, unsigned int y) const {
            assert(0 <= x && x < sideLength);
            assert(0 <= y && y < sideLength);

            bool bit0 = m_backing[x] & (0x01 << y);
            bool bit1 = m_backing[sideLength + x] & (0x01 << y);

            return ((u8_byte)bit1 << 1) & ((u8_byte)bit0);
        }

    private:
        u8_byte *m_backing;
    };

    class Sprite 
    {
    public:
        Sprite(u8_byte *backing,
               PPUController &controller) :
            m_backing (backing),
            m_controller (controller)
        {}
        ~Sprite() { m_backing = nullptr; }

        const static u8_byte VERTICAL_FLIP_MASK         = 0x80;
        const static u8_byte HORIZONTAL_FLIP_MASK       = 0x40;
        const static u8_byte BACKGROUND_PRIORITY_MASK   = 0x20;
        const static u8_byte SPRITE_PALETTE_MASK        = 0x03;

        unsigned int x() const { return m_backing[3]; }
        unsigned int y() const { return m_backing[0] - 1; }

        std::pair<unsigned int, unsigned int> size() const {
            return m_controller.spriteSize();
        }

        unsigned int numTiles() {
            return size().second >> 3;
        }

        u16_word tileAddress() {
            if (size().second == 8) {
                return m_controller.spritePatternTableAddress() + (tileSize * m_backing[1]);
            }
            return ((m_backing[1] & 0x01) * 0x1000) + ((m_backing[1] >> 1) * 2);
        }

        bool verticalFlip() const { return VERTICAL_FLIP_MASK & m_backing[2]; }
        bool horizontalFlip() const { return HORIZONTAL_FLIP_MASK & m_backing[2]; }
        bool isBehindBackground() const { return BACKGROUND_PRIORITY_MASK & m_backing[2]; }

        u8_byte paletteNumber() const { return SPRITE_PALETTE_MASK & m_backing[2]; }

    private:
        u8_byte       *m_backing;
        PPUController &m_controller;
    };

    bool m_NMI;

    Clock &m_clock;

    // PPU Control and Status Registers
    PPUController   m_control; 
    PPUMask         m_mask;
    PPUStatus       m_status;

    // PPU Object Attribute Memory registers
    OAMAddress      m_oamAddress;
    OAMData         m_oamData;
    OAMDMA          m_oamDMA;

    // PPU VRAM Access Registers
    VRAMScroll      m_scroll;
    VRAMAddress     m_address;      // (v) in "The skinny on NES scrolling"
    //VRAMAddress     m_temp_address; // (t) in "The skinny on NES scrolling"
    VRAMData        m_data;

    RegisterBlock   m_registerBlock;

    //1st write flip-flop/latch.
    bool            m_isFirstWrite;

    std::vector<Register*> m_registers;

    // PPU Memory
    Memory  *m_memory;
    Memory  *m_spriteRAM;

    // Rendering that we can display.
    float*   m_bitmap;

    unsigned int m_currentScanline;
    unsigned int m_currentCycle;
};

#endif
