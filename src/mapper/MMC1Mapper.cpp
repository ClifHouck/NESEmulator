#include "MMC1Mapper.hpp"

#include <cassert>

MMC1Mapper::
MMC1Mapper(iNESFile &file) :
    m_rom(file),
    m_shift(),
    m_configuration(),
    m_vromSelect0k(),
    m_vromSelect1k(),
    m_prgromSelect(),
    m_cpuMemory(PRG_RAM_BANK_BEGIN, 
                SECOND_PRG_ROM_BANK_END, 
                std::vector<Memory*>()),
    m_ppuMemory(FIRST_VROM_BANK_BEGIN,
                SECOND_VROM_BANK_END,
                std::vector<Memory*>()),
    m_prgRam(PRG_RAM_BANK_BEGIN, PRG_RAM_BANK_END),
    m_prgBanks(),
    m_vromBanks()
{
    // Init all necessary memory segments.

    // Copy valid data.
    for (unsigned int n = 0; n < m_rom.numberOfPRGROMPages(); ++n) {
        m_prgBanks.push_back(BackedMemory(PRG_BANK_SIZE, m_rom.prgRomPage(n)));
    }
    // Fill in rest of pages.
    for (unsigned int n = 0; n < (NUM_PRG_BANKS - m_rom.numberOfPRGROMPages()); ++n) {
        m_prgBanks.push_back(BackedMemory(PRG_BANK_SIZE));
    }

    // Copy valid data.
    for (unsigned int n = 0; n < NUM_VROM_BANKS; ++n) {
        m_prgBanks.push_back(BackedMemory(VROM_BANK_SIZE, m_rom.vromPage(n)));
    }
    // Fill in rest of pages.
    for (unsigned int n = 0; n < m_rom.numberOfCHRROMPages(); ++n) {
        m_vromBanks.push_back(BackedMemory(VROM_BANK_SIZE));
    }

    assert(m_prgBanks.size()  == NUM_PRG_BANKS);
    assert(m_vromBanks.size() == NUM_VROM_BANKS);

    // Add default segments.

    m_cpuMemory.addSegment(&m_prgRam);

    BackedMemory * segment = &(m_prgBanks.at(0));
    segment->setAddressRange(FIRST_PRG_ROM_BANK_BEGIN,
                             FIRST_PRG_ROM_BANK_END);
    m_cpuMemory.addSegment(segment);

    segment = &(m_prgBanks.at(1));
    segment->setAddressRange(SECOND_PRG_ROM_BANK_BEGIN,
                             SECOND_PRG_ROM_BANK_END);
    m_cpuMemory.addSegment(segment);

    // PPU Banks.
    segment = &(m_vromBanks.at(0));
    segment->setAddressRange(FIRST_VROM_BANK_BEGIN,
                             FIRST_VROM_BANK_END);
    m_ppuMemory.addSegment(segment);

    segment =&(m_vromBanks.at(1));
    segment->setAddressRange(SECOND_VROM_BANK_BEGIN,
                             SECOND_VROM_BANK_END);
    m_ppuMemory.addSegment(segment);
}

MMC1Mapper::
~MMC1Mapper()
{
}

Memory*
MMC1Mapper::
cpuMemory()
{
    return &m_cpuMemory;
}

Memory*
MMC1Mapper::
ppuMemory()
{
    return &m_ppuMemory;
}

const char *
MMC1Mapper::
name() const 
{
    return "MMC1";
}

void
MMC1Mapper::
updateMemory()
{
    // Update CPU memory map.

    // Remove the current banks.
    m_cpuMemory.removeSegment(FIRST_PRG_ROM_BANK_BEGIN);
    m_cpuMemory.removeSegment(SECOND_PRG_ROM_BANK_BEGIN);

    // Now add the appropriate banks back in based on register contents.
    u8_byte prgBankNumber = m_prgromSelect.bankNumber();
    BackedMemory * firstCpuBank  = nullptr;
    BackedMemory * secondCpuBank = nullptr;

    // Select the banks.
    switch(m_configuration.prgSwitchingMode()) {
        case ConfigurationRegister::Switchable32kArea0:
        case ConfigurationRegister::Switchable32kArea1:
            prgBankNumber   = prgBankNumber & 0x0E;
            firstCpuBank       = &(m_prgBanks.at(prgBankNumber));
            secondCpuBank      = &(m_prgBanks.at(prgBankNumber + 1));
            break;
        case ConfigurationRegister::Switchable16kAreaAtC000h:
            firstCpuBank  = &(m_prgBanks.at(0));
            secondCpuBank = &(m_prgBanks.at(prgBankNumber));
            break;
        case ConfigurationRegister::Switchable16kAreaAt8000h:
            firstCpuBank  = &(m_prgBanks.at(prgBankNumber)); 
            secondCpuBank = &(m_prgBanks.at(0x0F)); // Last bank
            break;
    }

    firstCpuBank->setAddressRange(FIRST_PRG_ROM_BANK_BEGIN,
                                  FIRST_PRG_ROM_BANK_END);
    secondCpuBank->setAddressRange(SECOND_PRG_ROM_BANK_BEGIN,
                                   SECOND_PRG_ROM_BANK_END);

    m_cpuMemory.addSegment(firstCpuBank);
    m_cpuMemory.addSegment(secondCpuBank);

    // Update PPU memory map.
    
    // Remove current PPU segments.
    m_ppuMemory.removeSegment(FIRST_VROM_BANK_BEGIN);
    m_ppuMemory.removeSegment(SECOND_VROM_BANK_BEGIN);

    BackedMemory * firstPpuBank     = nullptr;
    BackedMemory * secondPpuBank    = nullptr;

    // Switch 2 separate 4 KB banks.
    if (m_configuration.vromSwitchingSize()) {
        firstPpuBank    = &(m_vromBanks.at(m_vromSelect0k.bankNumber()));
        secondPpuBank   = &(m_vromBanks.at(m_vromSelect1k.bankNumber()));
    } 
    // Switch 8 KB bank.
    else {
        // We ignore the lowbit.
        u8_byte ppuBankSelect = m_vromSelect0k.bankNumber() & 0x1E;
        firstPpuBank          = &(m_vromBanks.at(ppuBankSelect));
        secondPpuBank         = &(m_vromBanks.at(ppuBankSelect + 1));
    }

    // Now add the select banks into the ppu memory map.
    m_ppuMemory.addSegment(firstPpuBank);
    m_ppuMemory.addSegment(secondPpuBank);
}

MMC1Mapper::CpuMemory::
CpuMemory(MMC1Mapper &parentMapper) :
    MappedMemory(PRG_RAM_BANK_BEGIN, SECOND_PRG_ROM_BANK_END, 
                 std::vector<Memory*>()),
    m_parentMapper (parentMapper)
{
}

Register*
MMC1Mapper::CpuMemory::
selectRegister(address_t address)
{
    if (address >= CONFIGURATION_REGISTER_RANGE_BEGIN &&
        address <= CONFIGURATION_REGISTER_RANGE_END) {
        return &m_parentMapper.m_configuration;
    }
    else if (address >= VROM_BANK_SELECT_0000_RANGE_BEGIN &&
             address <= VROM_BANK_SELECT_0000_RANGE_END) {
        return &m_parentMapper.m_vromSelect0k;
    }
    else if (address >= VROM_BANK_SELECT_1000_RANGE_BEGIN &&
             address <= VROM_BANK_SELECT_1000_RANGE_END) {
        return &m_parentMapper.m_vromSelect1k;
    }
    else if(address >= PRGROM_BANK_SELECT_RANGE_BEGIN &&
            address <= PRGROM_BANK_SELECT_RANGE_END) {
        return &m_parentMapper.m_prgromSelect;
    }
    assert(false && "MMC1Mapper::selectRegister: the address passed doesn't select any register!");
    return &m_parentMapper.m_prgromSelect;
}

void
MMC1Mapper::CpuMemory::
write(address_t address, u8_byte data)
{
    ShiftRegister &shift = m_parentMapper.m_shift;

    // If the reset bit is set, then we clear the shift register and
    // we're done.
    if (data & ShiftRegister::RESET_BIT_MASK) {
        shift.clear();
        return;
    }

    // Otherwise we push the data bit into the shift register.
    shift.pushBit(data & ShiftRegister::DATA_BIT_MASK);

    // And if it's full then we write out to the appropriate register, then
    // clear the shift register, and finish by updating the memory maps
    // as needed.
    if (shift.full()) {
        Register * reg = selectRegister(address);
        reg->write(shift.read());
        shift.clear();

        m_parentMapper.updateMemory();
    }
}

MMC1Mapper::ShiftRegister::
ShiftRegister() :
    Register(StateData(0x00, 0x00, 0x00, 0x00)),
    m_writeNum (0)
{}

void 
MMC1Mapper::ShiftRegister::
pushBit(bool bit)
{
    u8_byte data = rawRead();
    data = data << 1;  // Shift the current data left one.
    data = data | bit; // OR the data into the lowest bit.
    rawWrite(data);
    ++m_writeNum;
}

void 
MMC1Mapper::ShiftRegister::
clear()
{
    rawWrite(0x00);
    m_writeNum = 0;
}

bool 
MMC1Mapper::ShiftRegister::
full()
{
    return m_writeNum >= NUM_WRITES_TO_FILL;
}

MMC1Mapper::ConfigurationRegister::
ConfigurationRegister() :
    Register (StateData(0x00, 0x00, 0x00, 0x00))
{}

MMC1Mapper::ConfigurationRegister::NameTableMirroringType 
MMC1Mapper::ConfigurationRegister::
mirroringType() const
{
    switch (rawRead() & NAME_TABLE_MIRRORING_MASK)
    {
        case 0:
            return SingleScreenBlk0;
            break;
        case 1:
            return SingleScreenBlk1;
            break;
        case 2:
            return TwoScreenVerticalMirroring;
            break;
        case 3:
            return TwoScreenHorizontalMirroring;
            break;
        default:
            assert(false && "MMC1Mapper::ConfigurationRegister::mirroringType: This should never happen!");
            return SingleScreenBlk0;         
    }
}

MMC1Mapper::ConfigurationRegister::PrgSwitchingMode       
MMC1Mapper::ConfigurationRegister::
prgSwitchingMode() const
{
    u8_byte data = (rawRead() & PRG_SWITCHING_MODE_MASK) >> 2;
    switch (data) 
    {
        case 0:
            return Switchable32kArea0;
            break;
        case 1:
            return Switchable32kArea1;
            break;
        case 2:
            return Switchable16kAreaAtC000h;
            break;
        case 3:
            return Switchable16kAreaAt8000h;
            break;
        default:
            assert(false && "MMC1Mapper::ConfigurationRegister::prgSwitchingMode: This should never happen!");
            return Switchable32kArea0;
    }
}

bool                   
MMC1Mapper::ConfigurationRegister::
vromSwitchingSize() const
{
    return rawRead() & VROM_SWITCH_SIZE_MASK;
}
