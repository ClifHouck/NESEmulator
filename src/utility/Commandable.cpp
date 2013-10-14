#include "Commandable.hpp"

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

    // Find the target object, if there is one.
    MapType::iterator it = m_objects.find(name);

    if (it != m_objects.end()) {
        Commandable *reciever = it->second;
        result = reciever->recieveCommand(input);
    }

    return result;
}

bool
CommandDispatcher::
registerObject(Commandable* object)
{
    std::pair<MapType::iterator, bool> result;
    result = m_objects.insert(std::pair<std::string, Commandable*>(object->name(), object));
    return result.second;
}

bool
CommandDispatcher::
removeObject(Commandable* object)
{
    return m_objects.erase(object->name());
}
