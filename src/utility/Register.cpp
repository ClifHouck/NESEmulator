#include "Register.hpp"

Register::
Register(u8_byte initialData,
        u8_byte resetData) :
    PoweredDevice(this),
    m_data (initialData),
    m_resetData (resetData)
{
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
    m_data = m_data & mask;
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
    m_data = data & mask;
}
    
void 
Register::
resetImpl()
{
    m_data = m_resetData;
}

void
Register::
powerOnImpl() 
{
    m_data = m_resetData;
}

void
Register::
powerOffImpl()
{
}
