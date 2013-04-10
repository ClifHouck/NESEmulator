#include "Register.hpp"

Register::
Register(u8_byte initialData,
        u8_byte resetData) :
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
reset()
{
    PoweredDevice::reset();
    m_data = m_resetData;
}

void
Register::
powerOn() 
{
    PoweredDevice::powerOn();
    m_data = m_resetData;
}

void
Register::
powerOff()
{
    PoweredDevice::powerOff();
}
