#ifndef POWERED_DEVICE_H
#define POWERED_DEVICE_H

/* 
   Class for devices that respond to power events.
*/
class PoweredDevice
{
public:
    enum State { ON, OFF };

    PoweredDevice(State state = ON);

    virtual void reset(); 
    virtual void powerOn();
    virtual void powerOff();

    State state() const;

private:
    State m_state;
};

#endif //POWERED_DEVICE_H
