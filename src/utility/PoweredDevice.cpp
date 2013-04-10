#include "PoweredDevice::.hpp"

PoweredDevice::
PoweredDevice(State state = ON) :
    m_state (state)
{
}

void 
PoweredDevice::
reset() 
{
}

void 
PoweredDevice::
powerOn()
{
    m_state = ON;
}

void 
PoweredDevice::
powerOff()
{
    m_state = OFF;
}

PoweredDevice::State
PoweredDevice::
state() const
{
    return m_state;
}

