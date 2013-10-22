#include "Commandable.hpp"

#include <sstream>
#include <algorithm>
#include <iterator>

Commandable::
Commandable(std::string name) :
    m_name (name)
{
    // TODO: Some sort of name checking here?
    CommandDispatcher::instance()->delayedRegister(this);
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
    m_commands[command.m_code]       = command;
    m_translation[command.m_keyword] = command.m_code;
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

    if (m_awaitingRegistration.size() > 0) {
        finishRegistration();
    }

    // Parse the name from the input.
    // TODO: Strip leading whitespace.
    // TODO: More advanced argument passing.
    std::vector<std::string> tokens = parseArguments(input); 

    std::string name, command;
    if (tokens.size() >= 2) {
        name    = *tokens.begin();
        command = *(tokens.begin() + 1);
        tokens.erase(tokens.begin());
        tokens.erase(tokens.begin());
    }
    else {
        // We didn't get enough info...
        return result;
    }

    // Find the target object, if there is one.
    ObjectMapType::iterator it = m_objects.find(name);

    if (it != m_objects.end()) {
        Commandable *receiver = it->second;

        CommandInput comInput;
        comInput.m_keyword     = command;
        comInput.m_code        = receiver->translate(command);
        comInput.m_arguments   = tokens;

        result = receiver->receiveCommand(comInput);
    }

    return result;
}

void
CommandDispatcher::
delayedRegister(Commandable *object)
{
    m_awaitingRegistration.push_back(object);
}

void
CommandDispatcher::
finishRegistration()
{
    std::for_each(m_awaitingRegistration.begin(), 
                  m_awaitingRegistration.end(),
        [this] (Commandable * object) {
            registerObject(object);
    }); 
    m_awaitingRegistration.clear();
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

CommandDispatcher*
CommandDispatcher::
instance()
{
    static CommandDispatcher dispatcher;
    return &dispatcher;
}
