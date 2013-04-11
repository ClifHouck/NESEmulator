#ifndef POWERED_DEVICE_H
#define POWERED_DEVICE_H

/* 
   Interface class for devices that respond to power events.
*/
class PoweredDevice
{
public:
    enum State { ON, OFF };

    void reset();        
    void powerOn();     
    void powerOff();   

    State state() const;

protected:
    PoweredDevice(PoweredDevice *owner, State state = OFF);

    // Must be implemented in derived class.
    virtual void resetImpl()    = 0;
    virtual void powerOnImpl()  = 0;
    virtual void powerOffImpl() = 0;

private:
    PoweredDevice * m_poweredDeviceOwner;
    State           m_powerState;
};

#endif //POWERED_DEVICE_H
