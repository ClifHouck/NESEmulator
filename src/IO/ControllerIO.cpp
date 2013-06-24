#include "ControllerIO.hpp"

ControllerIO::
ControllerIO() :
    Memory(CONTROLLER_IO_BEGIN_ADDRESS, CONTROLLER_IO_END_ADDRESS),
    m_controller1(nullptr),
    m_controller2(nullptr),
    m_joypadInputRegister0(nullptr),
    m_joypadInputRegister1(nullptr),
    m_joypadOutputRegister()
{
}

NESController* 
ControllerIO::
controller1()
{
    return m_controller1;
}

NESController* 
ControllerIO::
controller2()
{
    return m_controller2;
}

void 
ControllerIO::
setController1(NESController *controller)
{
    m_controller1 = controller;
}

void 
ControllerIO::
setController2(NESController *controller)
{
    m_controller2 = controller;
}

data_t  
ControllerIO::
getData(address_t address)
{
    switch (address) 
    {
        case JOYPAD_INPUT_REGISTER_0_ADDRESS:
            return m_joypadInputRegister0.read();
            break;
        case JOYPAD_INPUT_REGISTER_1_ADDRESS:
            return m_joypadInputRegister1.read();
            break;
    };
}

void    
ControllerIO::
setData(address_t address, data_t data)
{
    switch (address)
    {
        case JOYPAD_OUTPUT_REGISTER_ADDRESS:
            m_joypadOutputRegister.write(data);
            break;
        case APU_LOW_FREQ_TIMER_CONTROL_ADDRESS:
            //TODO : Implement or warn.
            break;
    }
}

ControllerIO::JoypadInputRegister::
JoypadInputRegister(NESController *controller) :
    ReadOnlyRegister(StateData(0x00, 0x00, 0x00, 0x00)),
    m_controller (controller)
{
}

void
ControllerIO::JoypadInputRegister::
setController(NESController *controller)
{
    m_controller = controller;
}

u8_byte
ControllerIO::JoypadInputRegister::
read()
{
    u8_byte data = m_controller->read();
    m_controller->signalClock();
    return data;
}

NESJoypad::
NESJoypad() :
    NESController ()
{
}

void
NESJoypad::
signalClock()
{
    //TODO: Write current controller status to shift register.
}

u8_byte
NESJoypad::ShiftRegister::
read()
{
    // The shift register starts returning 1s 
    // if all the data has been read.
    if (goodbits == 0) {
        return 0x01;
    }

    u8_byte data = rawRead();
    rawWrite(data >> 1);
    goodbits = goodbits >> 1;
    return data & 0x01;
}

void
NESJoypad::ShiftRegister::
write(u8_byte data, u8_byte mask)
{
    m_goodbits = 0xFF;
    Register::write(data, mask);
}
