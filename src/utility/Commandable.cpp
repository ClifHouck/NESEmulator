#include "Commandable.hpp"

#include <sstream>
#include <algorithm>
#include <iterator>

Commandable::
Commandable(std::string name) :
    m_name (name)
{
    // TODO: Some sort of name checking here?
}

std::string
Commandable::
name() const
{
    return m_name;
}

void
Commandable::
setName(std::string name)
{
    // TODO: What happens in the dispatcher when we change an object's name?
    m_name = name;
}

void
Commandable::
addCommand(Command command)
{
    m_commands[command.m_code] = command;
}

Commandable::CommandMapType
Commandable::
commands()
{
    return m_commands;
}

CommandCode
Commandable::
translate(std::string keyword)
{
    TranslationMapType::iterator it = m_translation.find(keyword);
    if (it != m_translation.end()) {
        return it->second;
    }
    return UNRECOGNIZED_COMMAND;
}

std::vector<std::string>
CommandDispatcher::
parseArguments(std::string input)
{
    using namespace std;
    istringstream iss(input);
    vector<string> tokens;
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter<vector<string>>(tokens));
    return tokens;
}

CommandResult
CommandDispatcher::
command(const std::string& input)
{
    CommandResult result;
    result.m_code = CommandResult::NO_RECIEVER;

    // Parse the name from the input.
    // TODO: Strip leading whitespace.
    // TODO: More advanced argument passing.
    std::size_t position = input.find_first_of(" "); 
    std::string name = (position != std::string::npos) ? input.substr(0, position) : input;

    std::size_t end_command_position = input.find_first_of(" ", position);
    std::string command = input.substr(position + 1, end_command_position);

    // Find the target object, if there is one.
    ObjectMapType::iterator it = m_objects.find(name);

    if (it != m_objects.end()) {
        Commandable *reciever = it->second;

        CommandInput comInput;
        comInput.m_keyword     = command;
        comInput.m_code        = reciever->translate(command);
        comInput.m_arguments   = parseArguments(input);

        result = reciever->recieveCommand(comInput);
    }

    return result;
}

bool
CommandDispatcher::
registerObject(Commandable* object)
{
    std::pair<ObjectMapType::iterator, bool> result;

    // Register commands if needed.
    std::string type = object->typeName();
    CommandMapType::iterator it = m_commands.find(type);
    if (it == m_commands.end()) {
        m_commands[type] = object->commands();
    }

    // Register object.
    result = m_objects.insert(std::pair<std::string, Commandable*>(object->name(), object));
    return result.second;
}

bool
CommandDispatcher::
removeObject(Commandable* object)
{
    return m_objects.erase(object->name());
}
