#include "ControllerIO.hpp"

#include <cassert>

ControllerIO::
ControllerIO() :
    Memory(CONTROLLER_IO_BEGIN_ADDRESS, CONTROLLER_IO_END_ADDRESS),
    //FIXME: Remove this nullptr nonsense.
    m_controller1(nullptr),
    m_controller2(nullptr),
    m_joypadInputRegister0(nullptr),
    m_joypadInputRegister1(nullptr),
    m_joypadOutputRegister(nullptr, nullptr)
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
    m_joypadInputRegister0.setController(controller);
    m_joypadOutputRegister.setController1(controller);
}

void 
ControllerIO::
setController2(NESController *controller)
{
    m_controller2 = controller;
    m_joypadInputRegister1.setController(controller);
    m_joypadOutputRegister.setController2(controller);
}

Memory::data_t  
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
    assert(m_controller != nullptr && "JoypadInputRegister's m_controller member is nullptr!");
    u8_byte data = m_controller->read();
    m_controller->signalClock();
    return data;
}

NESJoypad::
NESJoypad() :
    NESController ()
{
}

NESJoypad::
~NESJoypad()
{}

void 
NESJoypad::
pressed(Button button)
{
    assert(button != ButtonsCount && "Invalid button value passed: ButtonsCount");
    m_pressed[button] = true;
}

void
NESJoypad::
signalClock()
{
    u8_byte pressedStatus = 0x00;
    int shiftAmount = 7;
    // Build the status byte.
    for (bool &is_pressed: m_pressed) { 
        pressedStatus = is_pressed << shiftAmount;
        // Clear the pressed status.
        is_pressed = false; 
        --shiftAmount;
    }

    m_shift.write(pressedStatus);
}

u8_byte
NESJoypad::
read()
{
    return m_shift.read();
}

NESJoypad::ShiftRegister::
ShiftRegister() :
    Register(Register::StateData(0x00, 0x00, 0x00, 0x00)),
    m_goodbits (0x00)
{}

u8_byte
NESJoypad::ShiftRegister::
read()
{
    // The shift register starts returning 1s 
    // if all the data has been read.
    if (m_goodbits == 0) {
        return 0x01;
    }

    u8_byte data = rawRead();
    rawWrite(data >> 1);
    m_goodbits = m_goodbits >> 1;
    return data & 0x01;
}

void
NESJoypad::ShiftRegister::
write(u8_byte data, u8_byte mask)
{
    m_goodbits = 0xFF;
    Register::write(data, mask);
}
