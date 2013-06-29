#ifndef CONTROLLER_IO_H
#define CONTROLLER_IO_H

#include "utility/DataTypes.hpp"
#include "utility/Memory.hpp"
#include "utility/Register.hpp"

class Controller;

class NESController
{
public:
    // TODO: Handle input from local host computer.

    virtual u8_byte read() = 0;
    virtual void signalClock() = 0;

    /* TODO: Do we need a pin-level emulation? Probably not...
    // Out pins
    virtual void signalClock() = 0;
    virtual bool out0() = 0;

    // In pins
    virtual void in0(bool input) = 0;
    virtual void in3(bool input) = 0;
    virtual void in4(bool input) = 0;
    */
};

class ControllerIO : public Memory
{
public:    
    ControllerIO();

    static const address_t JOYPAD_OUTPUT_REGISTER_ADDRESS       = 0x4016;
    static const address_t JOYPAD_INPUT_REGISTER_0_ADDRESS      = 0x4016;
    static const address_t JOYPAD_INPUT_REGISTER_1_ADDRESS      = 0x4017;
    static const address_t APU_LOW_FREQ_TIMER_CONTROL_ADDRESS   = 0x4017;

    static const address_t CONTROLLER_IO_BEGIN_ADDRESS          = JOYPAD_OUTPUT_REGISTER_ADDRESS;
    static const address_t CONTROLLER_IO_END_ADDRESS            = JOYPAD_INPUT_REGISTER_1_ADDRESS;

    NESController* controller1();
    NESController* controller2();

    void setController1(NESController *controller);
    void setController2(NESController *controller);

protected:
    virtual data_t  getData(address_t address);
    virtual void    setData(address_t address, data_t data);

private:
    class JoypadInputRegister : public ReadOnlyRegister
    {
    public:
        JoypadInputRegister(NESController * controller);

        void setController(NESController *controller);

        virtual u8_byte read();

    private:
        NESController *m_controller;
    };

    class JoypadOutputRegister : public WriteOnlyRegister
    {
    public:
        static const u8_byte STROBE_MASK = 0x01;

        JoypadOutputRegister(NESController * controller1, NESController * controller2) :
            WriteOnlyRegister(Register::StateData(0x00, 0x00, 0x00, 0x00)),
            m_controller1 (controller1),
            m_controller2 (controller2)
        {}

        void setController1(NESController * controller) { m_controller1 = controller; }
        void setController2(NESController * controller) { m_controller2 = controller; }

        virtual void write(u8_byte data, u8_byte mask = 0xFF) {
            //TODO: Handle output data to controllers and expansion port.
        }

    private:
        NESController * m_controller1;
        NESController * m_controller2;
    };

    // TODO: APU Low frequency timer control register.

    NESController *m_controller1; 
    NESController *m_controller2; 

    JoypadInputRegister     m_joypadInputRegister0;
    JoypadInputRegister     m_joypadInputRegister1;
    JoypadOutputRegister    m_joypadOutputRegister; 
};

class NESJoypad : public NESController 
{
public:
    NESJoypad();
    virtual ~NESJoypad();

    // NOTE: This list in order of what buttons will be read
    // through the controller's shift register.
    enum Button { 
        ButtonA, 
        ButtonB, 
        Select, 
        Start, 
        DirectionUp, 
        DirectionDown,
        DirectionLeft,
        DirectionRight,
        ButtonsCount // ALWAYS last element of enum.
    };

    void pressed(Button button);

    virtual void signalClock();

    virtual u8_byte read();

private:
    class ShiftRegister : public Register
    {
    public:
        ShiftRegister();

        virtual u8_byte read();
        virtual void    write(u8_byte data, u8_byte mask = 0xFF);

    private:
        u8_byte m_goodbits;
    };

    ShiftRegister m_shift;
    bool m_pressed[ButtonsCount];
};

#endif //CONTROLLER_IO_H
