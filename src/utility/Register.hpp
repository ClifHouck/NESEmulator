#ifndef REGISTER_H
#define REGISTER_H

#include "DataTypes.hpp"

#include "PoweredDevice.hpp"

class Register : public PoweredDevice
{
public:
    struct StateData
    {
        StateData(u8_byte powerOnData,
                  u8_byte resetData,
                  u8_byte readOnlyMask,
                  u8_byte resetMask) :
            m_powerOnData (powerOnData),
            m_resetData (resetData),
            m_readOnlyMask (readOnlyMask),
            m_resetMask (resetMask) 
        {}

        u8_byte m_powerOnData;
        u8_byte m_resetData;
        u8_byte m_readOnlyMask;
        u8_byte m_resetMask;
    };

    Register(StateData data);

    // Constructor for Register to use non-local backing.
    Register(u8_byte *backing,
             StateData data);

    // Functions for checked reads and writes.
    virtual u8_byte read();
    virtual void    write(u8_byte data, u8_byte mask = 0xFF);

protected:

    // Ignores rules like read-only masks and returns the raw data found in the backing store.
    // Use with caution.
    u8_byte rawRead() const;
    void    rawWrite(u8_byte data, u8_byte mask = 0xFF);

    // Interface required by PoweredDevice.
    void resetImpl();
    void powerOnImpl();
    void powerOffImpl();

private:
    u8_byte &m_data;
    u8_byte m_resetData;
    u8_byte m_readOnlyMask;
    u8_byte m_resetMask;
    u8_byte m_powerOnData;
    u8_byte m_localBacking;
};

class ReadOnlyRegister : public Register
{
public:
    ReadOnlyRegister(StateData data);

protected:
    virtual void write(u8_byte data, u8_byte mask = 0xFF);
};

class WriteOnlyRegister : public Register
{
public:
    WriteOnlyRegister(StateData data);

protected:
    virtual u8_byte read();
};

#endif //REGISTER_H
