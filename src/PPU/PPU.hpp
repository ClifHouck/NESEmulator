#ifndef PPU_H
#define PPU_H

#include "utility/DataTypes.hpp"
#include "utility/Register.hpp"
#include "utility/PoweredDevice.hpp"
#include "CPU/Cpu65XX.hpp"

#include <vector>

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
    bool m_NMI;

    unsigned int m_clock;

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
        u16_word spritePatternTableAddress() const { 
            return ((rawRead() & SPRITE_PATTERN_TABLE_ADDRESS_MASK) != 0) * 0x1000; 
        }
        u16_word backgroundPatternTableAddress() const {
            return ((rawRead() & BACKGROUND_PATTERN_TABLE__ADDRESS_MASK) != 0) * 0x1000;
        }
        std::pair<u8_byte, u8_byte> spriteSize() const {
            return std::pair<u8_byte, u8_byte>(8, 8 + (8 * (rawRead() & SPRITE_SIZE_MASK)));
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
        PPUStatus(u8_byte *backing) :
            Register(backing, 0xA0, 0x00, 0x00, 0x80)
        {}
        ~PPUStatus() {}

        static const u8_byte LEAST_SIGNIFICANT_BITS_MASK    = 0x1F;
        static const u8_byte SPRITE_OVERFLOW_MASK           = 0x20;
        static const u8_byte SPRITE_0_HIT_MASK              = 0x40;
        static const u8_byte VERTICAL_BLANK_STARTED_MASK    = 0x80;

        virtual u8_byte read() {
            rawWrite(rawRead() & ~VERTICAL_BLANK_STARTED_MASK);
            // TODO: Clear 'address latch'
            return Register::read();
        }

        bool spriteOverflow() { return rawRead() & SPRITE_OVERFLOW_MASK; }
        bool sprite0Hit() { return rawRead() & SPRITE_0_HIT_MASK; }
        bool verticalBlank() { return rawRead() & VERTICAL_BLANK_STARTED_MASK; }
    };

    class OAMAddress : public WriteOnlyRegister
    {
    public:
        OAMAddress(u8_byte *backing) :
            WriteOnlyRegister(backing, 0x00, 0x00, 0x00, 0xFF)
        {}
        ~OAMAddress() {}
    };

    class OAMData : public Register
    {
    public:
        OAMData(u8_byte *backing) :
            Register(backing, 0x00, 0x00, 0x00, 0x00)
        {}
        ~OAMData() {}
    };

    class PPUScroll : public WriteOnlyRegister
    {
    public:
        PPUScroll(u8_byte *backing) :
            WriteOnlyRegister(backing, 0x00, 0x00, 0x00, 0xFF)
        {}
        ~PPUScroll() {}
    };

    class PPUAddress : public WriteOnlyRegister
    {
    public:
        PPUAddress(u8_byte *backing) :
            WriteOnlyRegister(backing, 0x00, 0x00, 0x00, 0xFF),
            m_isHighWrite (true),
            m_highByte (0x00),
            m_lowByte (0x00)
        {}
        ~PPUAddress() {}

        u16_word address() const {
            return (m_highByte * 0x100) + m_lowByte;
        }

        virtual void write(u8_byte data, u8_byte mask = 0xFF) {
            WriteOnlyRegister::write(data, mask);
            if (m_isHighWrite) {
                m_highByte = rawRead();
            }
            else {
                m_lowByte = rawRead();
            }
            m_isHighWrite = !m_isHighWrite;
        }

    private:
        bool    m_isHighWrite;
        u8_byte m_highByte;
        u8_byte m_lowByte;
    };


    // PPU Control and Status Registers
    PPUController   m_control; 
    PPUMask         m_mask;
    PPUStatus       m_status;

    // PPU Object Attribute Memory registers
    OAMAddress      m_oamAddress;
    OAMData         m_oamData;

    // PPU VRAM Access Registers
    PPUScroll       m_scroll;
    PPUAddress      m_address;
    Register        m_data;

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
