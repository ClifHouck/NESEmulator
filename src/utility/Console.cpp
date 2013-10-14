#include "Console.hpp"

Console::
Console() :
    m_logger (Logger::get_instance()),
    m_dispatcher ()
{}

void
Console::
receive_input(std::string input)
{
    CommandResult result = m_dispatcher.command(input);

    *m_logger << result.m_output;
    // TODO: More output/handling here.
    if (result.m_code != CommandResult::OK) {
        *m_logger << result.m_meta;
    }
}

CommandDispatcher&
Console::
dispatcher()
{
    return m_dispatcher;
}
