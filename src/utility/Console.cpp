#include "Console.hpp"

#include "split.hpp"

#include <algorithm>

Console::
Console() :
    m_logger (Logger::get_instance()),
    m_dispatcher (CommandDispatcher::instance())
{
    m_logger->setOutStream(m_stream);
}

void
Console::
receive_input(std::string input)
{
    CommandResult result = m_dispatcher->command(input);

    *m_logger << "> " << input << "\n";
    *m_logger << result.m_output << "\n";
    // TODO: More output/handling here.
    if (result.m_code != CommandResult::OK) {
        *m_logger << result.m_meta << "\n";
    }
}

void
Console::
receive_script(std::string script)
{
    std::vector<std::string> lines = split(script, '\n');

    std::for_each(lines.begin(), lines.end(), [&](std::string line) {
            receive_input(line);
    });
}

std::string
Console::
contents()
{
    return m_stream.str();
}
