#ifndef REGISTER_H
#define REGISTER_H

#include "DataTypes.hpp"

#include "PoweredDevice.hpp"

class Register : public PoweredDevice
{
public:
    Register(u8_byte powerOnData    = 0x00,
             u8_byte resetData      = 0x00,
             u8_byte readOnlyMask   = 0x00,
             u8_byte resetMask      = 0x00);

    // Constructor for Register to use non-local backing.
    Register(u8_byte *backing,
             u8_byte powerOnData    = 0x00,
             u8_byte resetData      = 0x00,
             u8_byte readOnlyMask   = 0x00,
             u8_byte resetMask      = 0x00);

    // Functions for checked reads and writes.
    u8_byte read() const;
    void    write(u8_byte data, u8_byte mask = 0xFF);

    // Ignores rules like read-only masks and returns the raw data found in the backing store.
    u8_byte rawRead() const;
    void    rawWrite(u8_byte data, u8_byte mask = 0xFF);

protected:
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

#endif //REGISTER_H
