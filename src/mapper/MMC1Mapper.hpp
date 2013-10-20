#ifndef MMC1_MAPPER_H
#define MMC1_MAPPER_H

#include "Mapper.hpp"
#include "IO/iNESFile.hpp"
#include "utility/Register.hpp"

class MMC1Mapper : public Mapper
{
public:
    MMC1Mapper(iNESFile &file);
    virtual ~MMC1Mapper();

    virtual Memory *cpuMemory();
    virtual Memory *ppuMemory();

    virtual const char* name() const;

    // CPU Banks.
    static const Memory::address_t PRG_RAM_BANK_BEGIN           = 0x6000;
    static const Memory::address_t PRG_RAM_BANK_END             = 0x7FFF;
    static const Memory::address_t FIRST_PRG_ROM_BANK_BEGIN     = 0x8000;
    static const Memory::address_t FIRST_PRG_ROM_BANK_END       = 0xBFFF;
    static const Memory::address_t SECOND_PRG_ROM_BANK_BEGIN    = 0xC000;
    static const Memory::address_t SECOND_PRG_ROM_BANK_END      = 0xFFFF;

    // PPU Banks.
    static const Memory::address_t FIRST_VROM_BANK_BEGIN     = 0x0000;
    static const Memory::address_t FIRST_VROM_BANK_END       = 0x0FFF;
    static const Memory::address_t SECOND_VROM_BANK_BEGIN    = 0x1000;
    static const Memory::address_t SECOND_VROM_BANK_END      = 0x1FFF;

    // Basic bank info.
    static const unsigned int PRG_BANK_SIZE  = 16 * 1024;
    static const unsigned int NUM_PRG_BANKS  = 16;
    static const unsigned int VROM_BANK_SIZE = 4 * 1024;
    static const unsigned int NUM_VROM_BANKS = 32;

    // Write register address ranges.
    static const Memory::address_t CONFIGURATION_REGISTER_RANGE_BEGIN   = 0x8000;
    static const Memory::address_t CONFIGURATION_REGISTER_RANGE_END     = 0x9FFF;
    static const Memory::address_t VROM_BANK_SELECT_0000_RANGE_BEGIN    = 0xA000;
    static const Memory::address_t VROM_BANK_SELECT_0000_RANGE_END      = 0xBFFF;
    static const Memory::address_t VROM_BANK_SELECT_1000_RANGE_BEGIN    = 0xC000;
    static const Memory::address_t VROM_BANK_SELECT_1000_RANGE_END      = 0xDFFF;
    static const Memory::address_t PRGROM_BANK_SELECT_RANGE_BEGIN       = 0xE000;
    static const Memory::address_t PRGROM_BANK_SELECT_RANGE_END         = 0xFFFF;

private:
    // Makes cpu & ppu memory maps reflect current configuration.
    void  updateMemory();

    class CpuMemory : public MappedMemory
    {
    public:
        CpuMemory(MMC1Mapper &mapper);
        virtual ~CpuMemory() {}

        virtual void    write(address_t address, u8_byte data);

    private:
        Register * selectRegister(Memory::address_t address);

        MMC1Mapper & m_parentMapper;
    };

    class ShiftRegister : public Register
    {
    public:
        static const u8_byte DATA_BIT_MASK  = 0x01;
        static const u8_byte RESET_BIT_MASK = 0x80;
        static const unsigned int NUM_WRITES_TO_FILL = 5;

        ShiftRegister();
        virtual ~ShiftRegister() {}

        void pushBit(bool bit);
        void clear();

        bool full();

    private:
        unsigned int m_writeNum;
    };

    class ConfigurationRegister : public Register
    {
    public:
        ConfigurationRegister();
        ~ConfigurationRegister();

        static const u8_byte NAME_TABLE_MIRRORING_MASK = 0x03;
        static const u8_byte PRG_SWITCHING_MODE_MASK   = 0x0C;
        static const u8_byte VROM_SWITCH_SIZE_MASK     = 0x10;

        enum NameTableMirroringType {
            SingleScreenBlk0 = 0,
            SingleScreenBlk1 = 1,
            TwoScreenVerticalMirroring = 2,
            TwoScreenHorizontalMirroring = 3
        };

        enum PrgSwitchingMode {
            Switchable32kArea0 = 0,
            Switchable32kArea1 = 1,
            Switchable16kAreaAtC000h = 2,
            Switchable16kAreaAt8000h = 3
        };

        NameTableMirroringType mirroringType() const;
        PrgSwitchingMode       prgSwitchingMode() const;
        bool                   vromSwitchingSize() const;
    };

    class VROMSelectRegister : public Register
    {
    public:
        static const u8_byte VROM_BANK_SELECT_MASK = 0x1F;

        VROMSelectRegister() :
            Register (StateData("mmc1_vrom_select", 0x00, 0x00, 0x00, 0x00))
        {}

        virtual ~VROMSelectRegister() {}

        u8_byte bankNumber() const {
            return rawRead() & VROM_BANK_SELECT_MASK;
        }
    };

    class PRGROMSelectRegister : public Register
    {
    public:
        static const u8_byte ROM_BANK_SELECT_MASK = 0x0F;
        static const u8_byte RAM_DISABLE_MASK     = 0x10;

        PRGROMSelectRegister() :
            Register (StateData("mmc1_pgrom_select", 0x00, 0x00, 0x00, 0x00))
        {}
        virtual ~PRGROMSelectRegister() {}

        u8_byte bankNumber() const { return rawRead() & ROM_BANK_SELECT_MASK; }
        bool    ramDisable() const { return rawRead() & RAM_DISABLE_MASK; }
    };

    iNESFile              m_rom;

    //Registers
    ShiftRegister         m_shift;
    ConfigurationRegister m_configuration;  // Register 0
    VROMSelectRegister    m_vromSelect0k;   // Register 1 
    VROMSelectRegister    m_vromSelect1k;   // Register 2
    PRGROMSelectRegister  m_prgromSelect;   // Register 3

    MappedMemory m_cpuMemory;
    MappedMemory m_ppuMemory;
    BackedMemory m_prgRam;
    std::vector<BackedMemory> m_prgBanks;
    std::vector<BackedMemory> m_vromBanks;
};

#endif //MMC1_MAPPERH
