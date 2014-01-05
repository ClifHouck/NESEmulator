#include "NES.hpp"

const CommandCode RESET_COMMAND_CODE       = 0;
const CommandCode LOAD_ROM_COMMAND_CODE    = 1;

NES::
NES() :
    Commandable("nes"),
    PoweredDevice(this),
    m_clock (clockHertz),
    m_cpu (m_memory),
    m_ppu (&m_memory, m_clock),
    m_controllerIO (),
    m_memory (&m_ppu.registerBlock(),
              &m_controllerIO),
    m_mapper(nullptr)
{
    m_clock.registerDevice(&m_cpu);
    m_clock.registerDevice(&m_ppu);
    registerCommands();
}

NES::
~NES()
{
    delete m_mapper;
    m_mapper = nullptr;
}

void
NES::
load(const char * filename)
{
    iNESFile nesFile(filename);
    delete m_mapper;
    m_mapper = Mapper::getMapper(nesFile);

    // Load cartridge Memory objects into MainMemory and the PPU...
    Memory *cpuMemory = m_mapper->cpuMemory();
    m_memory.mappedMemory()->removeSegment(cpuMemory->startAddress());
    m_memory.mappedMemory()->addSegment(cpuMemory);

    m_ppu.setCartridgeMemory(m_mapper->ppuMemory());
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
MainMemory(Memory *ppuRegisters,
           Memory *controllerIO) :
    Memory(WORK_RAM_BEGIN, CARTRIDGE_PRGROM_END),
    m_mappedMemory (nullptr),
    m_workRam (WORK_RAM_BEGIN, WORK_RAM_END),
    m_apuRam  (APU_REGISTERS_BEGIN, APU_REGISTERS_END),
    m_cartridgeRam (CARTRIDGE_EXPO_BEGIN, CARTRIDGE_PRGROM_END)
{
    assert(ppuRegisters != nullptr && "ppuRegisters is nullptr!");
    assert(controllerIO != nullptr && "controllerIO is nullptr!");

    std::vector<Memory*> segments;
    segments.push_back(&m_workRam);
    segments.push_back(ppuRegisters);
    segments.push_back(&m_apuRam);
    segments.push_back(&m_cartridgeRam);
    segments.push_back(controllerIO);
    m_mappedMemory = new MappedMemory(WORK_RAM_BEGIN, CARTRIDGE_PRGROM_END, segments);
    assert(m_mappedMemory != nullptr);
}

NES::MainMemory::
~MainMemory() 
{
    delete m_mappedMemory;
    m_mappedMemory = nullptr; 
}

Memory::address_t
NES::MainMemory::
correctAddress(address_t address) const
{
    if (address >  WORK_RAM_END &&
        address <= WORK_RAM_MIRROR_END) {
        return address % WORK_RAM_SIZE;
    }
    return address;
}

NES::MainMemory::data_t
NES::MainMemory::
getData(address_t address) 
{
    return m_mappedMemory->read(correctAddress(address));
}

void
NES::MainMemory::
setData(address_t address, data_t data) 
{
    return m_mappedMemory->write(correctAddress(address), data);
}

void
NES::
registerCommands()
{
    // TODO PAUSE
    // TODO CONTINUE

    Command reset;
    reset.m_keyword         = "reset";
    reset.m_code            = RESET_COMMAND_CODE;
    reset.m_helpText        = "Resets the NES.";
    reset.m_numArguments    = 0;
    addCommand(reset);

    Command loadRom;
    loadRom.m_keyword   = "load";
    loadRom.m_code      = LOAD_ROM_COMMAND_CODE;
    loadRom.m_helpText  = "Takes 1 argument: The file to load.\n"
                          "    Load a ROM into the NES. Causes NES to reset.";
    loadRom.m_numArguments = 1;
    addCommand(loadRom);

    // TODO POWER ON / OFF 
}

CommandResult 
NES::
receiveCommand(CommandInput command)
{
    CommandResult result;
    result.m_code = CommandResult::NO_RECEIVER;

    switch(command.m_code) {
            // TODO PAUSE
            // TODO CONTINUE
            // TODO RESET
            case RESET_COMMAND_CODE:
            {
                reset();
                result.m_code = CommandResult::OK;
            }
            break;
            // TODO LOAD ROM
            case LOAD_ROM_COMMAND_CODE:
            {
                if (command.m_arguments.size() < 1) {
                    result.m_code = CommandResult::WRONG_NUM_ARGS;
                    result.m_meta = std::string("No filename specified.");
                    return result;
                }
                std::string filename = *command.m_arguments.begin();            
                load(filename.c_str());
                reset();
                result.m_code = CommandResult::OK;
            }
            break;
            // TODO POWER ON / OFF 
    }

    return result;
}
