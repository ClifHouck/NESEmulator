#ifndef CLOCK_H
#define CLOCK_H

#include <vector>

class ClockedDevice
{
public:
    ClockedDevice(unsigned int divisor);

    virtual void tick() = 0;

    unsigned int divisor();

private:
    unsigned int m_divisor;
};

class Clock
{
public:
    Clock(unsigned int hertz);

    void registerDevice(ClockedDevice *device);

    void tick();

    unsigned int count() const;

private:
    std::vector<ClockedDevice*> m_devices;
    unsigned int                m_count;
    unsigned int                m_hertz;
};

#endif //CLOCK_H
