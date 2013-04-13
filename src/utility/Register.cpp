#include "Register.hpp"

Register::
Register(u8_byte powerOnData,
        u8_byte resetData,
        u8_byte readOnlyMask,   
        u8_byte resetMask) :
    PoweredDevice(this),
    m_data (m_localBacking),
    m_resetData (resetData),
    m_readOnlyMask (readOnlyMask),
    m_resetMask (resetMask),
    m_powerOnData (powerOnData),
    m_localBacking (powerOnData)
{
}

Register::
Register(u8_byte *backing,
        u8_byte powerOnData,
        u8_byte resetData,
        u8_byte readOnlyMask,   
        u8_byte resetMask) :
    PoweredDevice(this),
    m_data (*backing),
    m_resetData (resetData),
    m_readOnlyMask (readOnlyMask),
    m_resetMask (resetMask),
    m_powerOnData (powerOnData)
{
    m_data = powerOnData;
}

u8_byte 
Register::
read() const
{
    return m_data;
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
    m_data = (data & mask) | (m_data & ~mask);
}

u8_byte 
Register::
rawRead() const
{
    return m_data;
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
