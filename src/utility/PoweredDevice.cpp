#include "PoweredDevice.hpp"

#include <cassert>
#include <cstddef>

PoweredDevice::
PoweredDevice(PoweredDevice *owner, State state) :
    m_poweredDeviceOwner (owner),
    m_powerState (state)
{
    // FIXME: For some reason clang is choking on the use of nullptr here.
    // assert(owner != nullptr);
    assert(owner != 0);
}

void 
PoweredDevice::
reset() 
{
    if (m_powerState == ON) {
        m_poweredDeviceOwner->resetImpl();
    }
}

void 
PoweredDevice::
powerOn()
{
    m_powerState = ON;
    m_poweredDeviceOwner->powerOnImpl();
}

void 
PoweredDevice::
powerOff()
{
    m_powerState = OFF;
    m_poweredDeviceOwner->powerOffImpl();
}

PoweredDevice::State
PoweredDevice::
powerState() const
{
    return m_powerState;
}

