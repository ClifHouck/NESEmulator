#include "Register.hpp"

#include <sstream>

CommandCode READ_COMMAND_CODE   = 0;
CommandCode WRITE_COMMAND_CODE  = 1;

Register::
Register(StateData data) :     
    PoweredDevice(this),
    Commandable(data.m_name),
    m_data (m_localBacking),
    m_resetData (data.m_resetData),
    m_readOnlyMask (data.m_readOnlyMask),
    m_resetMask (data.m_resetMask),
    m_powerOnData (data.m_powerOnData),
    m_localBacking (data.m_powerOnData)
{
    registerCommands();
}

Register::
Register(u8_byte *backing,
         StateData data) :
    PoweredDevice(this),
    Commandable(data.m_name),
    m_data (*backing),
    m_resetData (data.m_resetData),
    m_readOnlyMask (data.m_readOnlyMask),
    m_resetMask (data.m_resetMask),
    m_powerOnData (data.m_powerOnData)
{
    m_data = data.m_powerOnData;
    registerCommands();
}

u8_byte 
Register::
read() 
{
    return rawRead();
}

void    
Register::
write(u8_byte data, u8_byte mask)
{
    // Modify the mask so that it only touches bits that are not 
    // read-only.
    // TODO: Generate a warning when a caller tries to modify read-only
    // bits. That is, trying to write a 1 to a 0 or a 0 to a 1.
    mask &= ~m_readOnlyMask;
    rawWrite((data & mask) | (m_data & ~mask));
}

u8_byte 
Register::
rawRead() const
{
    return m_data;
}

void 
Register::
registerCommands()
{
    Command read;
    read.m_keyword = "read";
    read.m_code    = READ_COMMAND_CODE;
    read.m_helpText = "Print the current value of the register.";
    read.m_numArguments = 0;
    addCommand(read);

    Command write;
    write.m_keyword = "write";
    write.m_code    = WRITE_COMMAND_CODE;
    write.m_helpText = "Write an 8-bit hexadecimal value to the register.";
    write.m_numArguments = 1;
    addCommand(write);
}

CommandResult        
Register::
recieveCommand(CommandInput input)
{
    CommandResult result;
    result.m_code = CommandResult::ResultCode::NO_RECIEVER;
    switch (input.m_code) {
        READ_COMMAND_CODE:
        {
            std::stringstream output;    
            output << "0x" << std::hex << rawRead();
            result.m_output = output.str();
            result.m_code   = CommandResult::ResultCode::OK;
        }
        break;
        WRITE_COMMAND_CODE:
        //TODO...
        break;
    }
    return result;
}

void    
Register::
rawWrite(u8_byte data, u8_byte mask)
{
    m_data = (data & mask) | (m_data & ~mask);
}
    
void 
Register::
resetImpl()
{
    // Save the bits that are part of the mask, while adding the resetValue
    // bits which are NOT part of the mask.
    m_data = (m_data & m_resetMask) | (m_resetData & ~m_resetMask);
}

void
Register::
powerOnImpl() 
{
    m_data = m_powerOnData;
}

void
Register::
powerOffImpl()
{
}

ReadOnlyRegister::
ReadOnlyRegister(StateData data) :
    Register(data)
{
}

void 
ReadOnlyRegister::
write(u8_byte data, u8_byte mask)
{
    // TODO print a warning message to a log that an attempt to write to
    // a read-only register occurred. 
}

WriteOnlyRegister::
WriteOnlyRegister(StateData data) :
    Register(data)
{}

u8_byte 
WriteOnlyRegister::
read()
{
    // TODO log a warning message that an attempt was made to read a 
    // write only register
    return 0x00;
}
