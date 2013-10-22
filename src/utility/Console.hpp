#ifndef CONSOLE_H
#define CONSOLE_H

#include "Commandable.hpp"
#include "Logger.hpp"

#include <map>
#include <string>

/*
Console

A way to interact with the application.  Outputs the result to the logger.
*/

class Console 
{
public:
    Console();

    void receive_input(std::string input);

private:
    CommandDispatcher  *m_dispatcher;
    Logger             *m_logger;
};

#endif //CONSOLE_H
