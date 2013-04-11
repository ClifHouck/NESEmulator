#ifndef REGISTER_H
#define REGISTER_

#include "DataTypes.hpp"

#include "PoweredDevice.hpp"

class Register : public PoweredDevice
{
public:
    Register(u8_byte initialData,
             u8_byte resetData);

    // Functions for checked reads and writes.
    u8_byte read() const;
    void    write(u8_byte data, u8_byte mask);

    // Ignores rules like read-only masks and returns the raw data found in the backing store.
    u8_byte rawRead() const;
    void    rawWrite(u8_byte data, u8_byte mask);

protected:
    // Interface required by PoweredDevice interface.
    void resetImpl();
    void powerOnImpl();
    void powerOffImpl();

private:
    u8_byte m_data;
    u8_byte m_resetData;
};

#endif //REGISTER_H
