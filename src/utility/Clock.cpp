#include "Clock.hpp"

#include <algorithm>

Clock::
Clock(unsigned int hertz) :
    m_hertz (hertz),
    m_count (0)
{}

void 
Clock::
registerDevice(ClockedDevice *device)
{
    m_devices.push_back(device);
    //TODO: Code to layout devices in a list by time?
}

void 
Clock::
tick()
{
    std::for_each(m_devices.begin(), m_devices.end(), [this](ClockedDevice *device) {
            // TODO: Modulus is costly, is there a better way to do this?
            // Like, finding the greatest common divisor and then laying out 
            // a list of devices by time in which they need to be ticked and
            // then just constantly running through that circular list?
            if (m_count % device->divisor() == 0) {
                device->tick();
            }
    });

    //TODO: Measure time elapsed between ticks, and make sure it matches the specific hertz (if possible).

    m_count++;
}

unsigned int
Clock::
count() const
{
    return m_count;
}

ClockedDevice::
ClockedDevice(unsigned int divisor) :
    m_divisor (divisor)
{
}

unsigned int 
ClockedDevice::
divisor()
{
    return m_divisor;
}
