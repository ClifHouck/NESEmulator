#include <string>

#include "NesApp.hpp"

int main(int argc, char** argv)
{
    std::string script_name;
    if (argc == 2) {
        script_name = std::string(argv[1]);
    }

    NESApp nes(script_name);

    return nes.onExecute();
}
