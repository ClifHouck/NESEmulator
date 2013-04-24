#ifndef PPU_H
#define PPU_H

#include "utility/DataTypes.hpp"
#include "utility/Register.hpp"
#include "utility/PoweredDevice.hpp"
#include "CPU/Cpu65XX.hpp"

#include <vector>
#include <cassert>

class PPU : public PoweredDevice
{
public:
    PPU(Cpu65XX::Memory& cpuMemory);
    ~PPU();

    const static unsigned int width             = 256;
    const static unsigned int height            = 240;
    const static unsigned int ticksPerScanline  = 341;
    const static unsigned int memorySize        = 16 * 1024;
    const static unsigned int spriteRamSize     = 256;
    const static unsigned int bitmapSize        = width * height * 3;
    const static unsigned int spriteSize        = 4;
    const static unsigned int tileSize          = 16;

    // FIXME: Remove this and make a general memory class.
    class Memory {
        public:
            Memory();
            Memory(u8_byte * toLoad, unsigned int size);

            u8_byte& byteAt(const u16_word& address);

        private:
            u8_byte m_memory[memorySize];
    };

    void tick();

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

    class PPUController : public ReadOnlyRegister
    {
    public:
        PPUController(u8_byte *backing) :
            ReadOnlyRegister(backing, 0x00, 0x00, 0x00, 0x00)
        {}
        ~PPUController() {}

        static const u8_byte BASE_NAMETABLE_ADDRESS_MASK            = 0x03;
        static const u8_byte VRAM_ADDRESS_INCREMENT_MASK            = 0x04;
        static const u8_byte SPRITE_PATTERN_TABLE_ADDRESS_MASK      = 0x08;
        static const u8_byte BACKGROUND_PATTERN_TABLE__ADDRESS_MASK = 0x10;
        static const u8_byte SPRITE_SIZE_MASK                       = 0x20;
        static const u8_byte MASTER_SLAVE_SELECT_MASK               = 0x40;
        static const u8_byte GENERATE_NMI_MASK                      = 0x80;

        u16_word baseNameTableAddress() const {
            return 0x2000 + ((rawRead() & BASE_NAMETABLE_ADDRESS_MASK) != 0) * 0x400;
        }
        bool vramAddressIncrement() const { return rawRead() & VRAM_ADDRESS_INCREMENT_MASK; }

        int getAddressIncrementAmount() const { 
            if (vramAddressIncrement()) {
                return 32;
            }
            return 1;
        }

        u16_word spritePatternTableAddress() const { 
            return ((rawRead() & SPRITE_PATTERN_TABLE_ADDRESS_MASK) != 0) * 0x1000; 
        }
        u16_word backgroundPatternTableAddress() const {
            return ((rawRead() & BACKGROUND_PATTERN_TABLE__ADDRESS_MASK) != 0) * 0x1000;
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
        PPUMask(u8_byte *backing) :
            Register(backing, 0x00, 0x00, 0x00, 0x00)
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
        PPUStatus(u8_byte *backing,
                  bool    &isFirstWrite) :
            Register(backing, 0xA0, 0x00, 0x00, 0x80),
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
        OAMAddress(u8_byte *backing) :
            WriteOnlyRegister(backing, 0x00, 0x00, 0x00, 0xFF)
        {}
        ~OAMAddress() {}

        void increment() { rawWrite(rawRead() + 1); }

        u8_byte address() const { return rawRead(); }
    };

    class OAMData : public Register
    {
    public:
        OAMData(u8_byte *backing,
                u8_byte *spriteRAM,
                OAMAddress &address) :
            Register(backing, 0x00, 0x00, 0x00, 0x00),
            m_spriteRAM (spriteRAM),
            m_address(address)
        {}
        ~OAMData() {
            m_spriteRAM = nullptr;
        }

        virtual u8_byte read() {
            rawWrite(m_spriteRAM[m_address.address()]);
            return Register::read();
        }

        virtual void write(u8_byte data, u8_byte mask = 0xFF) {
            Register::write(data, mask);
            m_spriteRAM[m_address.address()] = rawRead();
            m_address.increment();
        }

    private:
        u8_byte    *m_spriteRAM;
        OAMAddress &m_address;
    };

    class OAMDMA : public WriteOnlyRegister 
    {
    public:
        OAMDMA(u8_byte *backing) :
            WriteOnlyRegister(backing, 0x00, 0x00, 0x00, 0x00)
        {}
        ~OAMDMA() {}

        u16_word address() const { return rawRead() * 0x100; }
    };

    class VRAMScroll : public WriteOnlyRegister
    {
    public:
        VRAMScroll(u8_byte *backing,
                   bool &isFirstWrite) :
            WriteOnlyRegister(backing, 0x00, 0x00, 0x00, 0xFF),
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

    class VRAMAddress : public WriteOnlyRegister
    {
    public:
        VRAMAddress(u8_byte *backing,
                    bool &isFirstWrite,
                    PPUController &ppuController) :
            WriteOnlyRegister(backing, 0x00, 0x00, 0x00, 0xFF),
            m_isFirstWrite (isFirstWrite),
            m_ppuController (ppuController),
            m_highByte (0x00),
            m_lowByte (0x00)
        {}
        ~VRAMAddress() {}

        u16_word address() const {
            return (m_highByte * 0x100) + m_lowByte;
        }

        virtual void write(u8_byte data, u8_byte mask = 0xFF) {
            WriteOnlyRegister::write(data, mask);
            if (m_isFirstWrite) {
                m_highByte = rawRead();
            }
            else {
                m_lowByte = rawRead();
            }
            m_isFirstWrite = !m_isFirstWrite;
        }

        void increment() {
            rawWrite(rawRead() + m_ppuController.getAddressIncrementAmount());
        }

    private:
        PPUController  &m_ppuController;
        bool           &m_isFirstWrite;
        u8_byte         m_highByte;
        u8_byte         m_lowByte;
    };

    class VRAMData : public Register 
    {
    public:
        VRAMData(u8_byte *backing, 
                 VRAMAddress &vramAddress,
                 Cpu65XX::Memory& cpuMemory) :
            Register(backing, 0x00, 0x00, 0x00, 0x00),
            m_vramAddress (vramAddress),
            m_cpuMemory (cpuMemory)
        {}
        ~VRAMData() {}

        virtual u8_byte read() {
            // Load the data from CPU memory into this register.
            Register::rawWrite(m_cpuMemory.byteAt(m_vramAddress.address()));
            m_vramAddress.increment();
            return Register::read();
        }
        
        virtual void write(u8_byte data, u8_byte mask = 0xFF) {
            u8_byte &VRAMTarget = m_cpuMemory.byteAt(m_vramAddress.address());
            // Update the contents of this register.
            rawWrite(VRAMTarget);
            Register::write(data, mask);
            // Write back to the CPU's memory.
            VRAMTarget = rawRead();
            m_vramAddress.increment();
        }

    private:
        VRAMAddress         &m_vramAddress;
        Cpu65XX::Memory     &m_cpuMemory; 
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
            assert(0 <= x < sideLength);
            assert(0 <= y < sideLength);

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

    unsigned int m_clock;

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
    VRAMAddress     m_address;
    VRAMData        m_data;

    //1st write flip-flop/latch.
    bool            m_isFirstWrite;

    std::vector<Register*> m_registers;

    // PPU Memory
    Memory  m_memory;
    u8_byte m_spriteRAM[spriteRamSize];

    // Rendering that we can display.
    float   m_bitmap[bitmapSize];

    unsigned int m_currentScanline;
    unsigned int m_currentCycle;
};

#endif
