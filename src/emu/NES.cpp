#include "NES.hpp"

#include <algorithm>

const CommandCode RESET_COMMAND_CODE       = 0;
const CommandCode LOAD_ROM_COMMAND_CODE    = 1;
const CommandCode PAUSE_COMMAND_CODE       = 2;
const CommandCode CONTINUE_COMMAND_CODE    = 3;

NES::
NES() :
    Commandable("nes"),
    PoweredDevice(this),
    m_clock (clockHertz),
    m_cpu (m_memory),
    m_ppu (&m_memory, m_clock),
    m_controllerIO (),
    m_memory (nullptr, nullptr),
    m_mapper (nullptr),
    m_paused (true)
{
    m_memory = MainMemory(&m_ppu.registerBlock(), &m_controllerIO);
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
    MappedMemory *mappedMemory = m_memory.mappedMemory();
    mappedMemory->removeSegment(cpuMemory->startAddress());
    mappedMemory->addSegment(cpuMemory);

    m_ppu.setCartridgeMemory(m_mapper->ppuMemory());
}

void 
NES::
tick() 
{
    if (m_paused) { return; }
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
    // TODO: Invert this so the PPU draws on its registers from the main memory pool rather than
    // passing its own memory to this object.
    /* assert(ppuRegisters != nullptr && "ppuRegisters is nullptr!");
       assert(controllerIO != nullptr && "controllerIO is nullptr!"); */

    std::vector<Memory*> segments;
    segments.push_back(&m_workRam);
    segments.push_back(ppuRegisters);
    segments.push_back(&m_apuRam);
    segments.push_back(&m_cartridgeRam);
    // FIXME: APU & ControllerIO share some registers...
    segments.push_back(controllerIO);
    m_mappedMemory = new MappedMemory(WORK_RAM_BEGIN, CARTRIDGE_PRGROM_END, segments);
    assert(m_mappedMemory != nullptr);
}

NES::MainMemory::
MainMemory(const MainMemory& other) :
    Memory (other.m_startAddress, other.m_endAddress),
    m_workRam (other.m_workRam),
    m_apuRam  (other.m_apuRam),
    m_cartridgeRam (other.m_cartridgeRam)
{
    m_mappedMemory = dynamic_cast<MappedMemory*>(other.m_mappedMemory->clone());
    assert(m_mappedMemory != nullptr);
}

NES::MainMemory::
~MainMemory() 
{
    if (m_mappedMemory != nullptr) {
        delete m_mappedMemory;
        m_mappedMemory = nullptr; 
    }
}

NES::MainMemory&
NES::MainMemory::
operator=(MainMemory tmp)
{
    std::swap(m_workRam,      tmp.m_workRam); 
    std::swap(m_apuRam,       tmp.m_apuRam); 
    std::swap(m_cartridgeRam, tmp.m_cartridgeRam); 
    std::swap(m_mappedMemory, tmp.m_mappedMemory);

    return *this;
}

Memory::address_t
NES::MainMemory::
correctAddress(address_t address) const
{
    // TODO: Modulo is expensive...
    if (address >  WORK_RAM_END &&
        address <= WORK_RAM_MIRROR_END) {
        return address % WORK_RAM_SIZE; 
    }
    else if (address >  PPU_REGISTERS_END &&
             address <= PPU_MIRROR_END) {
        return PPU_REGISTERS_BEGIN + (address % PPU_REGISTERS_SIZE); 
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
    // TODO: 
    // POWER ON / OFF 
    // Step instruction
    // Watchpoints
    // Breakpoints
    std::vector<Command> commands = {
        { "pause",    PAUSE_COMMAND_CODE,    "Pauses execution of the NES.", 0},
        { "continue", CONTINUE_COMMAND_CODE, "Continues execution of the NES.", 0 },
        { "reset",    RESET_COMMAND_CODE,    "Resets the NES.", 0 },
        { "load",     LOAD_ROM_COMMAND_CODE, "Takes 1 argument: The file to load.\n"
                                             " Load a ROM into the NES. Causes NES to reset.", 1}
    };

    std::for_each(commands.begin(), commands.end(), [&](Command c) { addCommand(c); });
}

CommandResult 
NES::
receiveCommand(CommandInput command)
{
    CommandResult result;
    result.m_code = CommandResult::NO_RECEIVER;

    switch(command.m_code) {
            case PAUSE_COMMAND_CODE:
            {
                m_paused = true;
                result.m_code = CommandResult::OK;
            }
            break;
            case CONTINUE_COMMAND_CODE:
            {
                m_paused = false;
                result.m_code = CommandResult::OK;
            }
            break;
            case RESET_COMMAND_CODE:
            {
                reset();
                result.m_code = CommandResult::OK;
            }
            break;
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
