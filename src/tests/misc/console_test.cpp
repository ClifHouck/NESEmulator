#include "utility/Console.hpp"
#include "utility/Register.hpp"

int main(int argc, char ** argv) 
{
    Register reg(Register::StateData("test_reg", 0x00, 0x00, 0x00, 0x00));
    Console console;
    
    console.receive_input("test_reg read");
    console.receive_input("test_reg write 255");
    console.receive_input("test_reg read");
    console.receive_input("test_reg write 53");
    console.receive_input("test_reg read");
    console.receive_input("test_reg write");
    console.receive_input("test_reg write asdf");

    return 0;
}
