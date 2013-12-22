#ifndef CONSOLE_H
#define CONSOLE_H

#include "Commandable.hpp"
#include "Logger.hpp"

#include <map>
#include <string>
#include <sstream>

/*
Console

A way to interact with the application.  Outputs the result to the logger.
*/

class Console 
{
public:
    Console();

    void receive_input(std::string input);
    std::string contents();

private:
    std::stringstream  m_stream;
    CommandDispatcher  *m_dispatcher;
    Logger             *m_logger;
};

#endif //CONSOLE_H
