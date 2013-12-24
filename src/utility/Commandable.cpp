#include "Commandable.hpp"

#include <sstream>
#include <algorithm>
#include <iterator>

#include <iostream>

Commandable::
Commandable(std::string name) :
    m_name (name)
{
    // TODO: Some sort of name checking here?
    CommandDispatcher::instance()->delayedRegister(this);

    Command help_command;
    help_command.m_keyword  = "help";
    help_command.m_helpText = "List the commands that this object can recieve and how to use them.";
    help_command.m_code     = HELP_COMMAND;
    std::cout << "Help code is " << HELP_COMMAND << "\n";
    help_command.m_numArguments = 0;
    addCommand(help_command);
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

std::string
Commandable::
help()
{
    std::stringstream output;

    std::for_each(m_commands.begin(), m_commands.end(), [this, &output] (CommandMapType::value_type &value) {
            output << value.second.m_keyword << " - " << value.second.m_helpText << "\n";
    });

    return output.str();
}

CommandResult
Commandable::
receiveBuiltInCommand(CommandInput command)
{
    CommandResult result;

    switch (command.m_code) {
        case HELP_COMMAND:
            result.m_code = CommandResult::ResultCode::OK;
            result.m_output = help();
            break;
    }

    return result;
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
handleBuiltInCommand(std::vector<std::string> tokens)
{
    CommandResult result;

    if (tokens.size() == 0) { return result; }

    std::string command = *tokens.begin();
    tokens.erase(tokens.begin());

    if (command == std::string("list")) {
        std::string name_of_type = *tokens.begin();

        ObjectTypeNameToVectorType::iterator it = m_typelist.find(name_of_type);
        if (it == m_typelist.end()) {
            std::stringstream output;
            output << "There is no known object of type '" <<  name_of_type << "'.";
            result.m_meta = output.str();
            result.m_code = CommandResult::ResultCode::ERROR;
            return result;
        }

        std::stringstream output;
        std::for_each(it->second.begin(), it->second.end(), [&output] (Commandable *obj) {
            output << obj->name() << std::endl;
        });
        result.m_output = output.str();
    }

    return result;
}

CommandResult
CommandDispatcher::
command(const std::string& input)
{
    CommandResult result;
    result.m_code = CommandResult::NO_RECEIVER;

    if (m_awaitingRegistration.size() > 0) {
        finishRegistration();
    }

    // Parse the name from the input.
    // TODO: Strip leading whitespace.
    // TODO: More advanced argument passing.
    std::vector<std::string> tokens = parseArguments(input); 

    // First try to handle the command as a built-in.
    result = handleBuiltInCommand(tokens);
    if (result.m_code != CommandResult::ResultCode::NO_RECEIVER) {
        return result;
    }

    std::string name, command;
    if (tokens.size() >= 2) {
        name    = *tokens.begin();
        command = *(tokens.begin() + 1);
        tokens.erase(tokens.begin());
        tokens.erase(tokens.begin());
    }
    else {
        result.m_meta = std::string("Form of command is incorrect. Try <object_name> <command> <arguments>...");
        return result;
    }

    // Otherwise, try to find an object to receive the command.
    ObjectMapType::iterator it = m_objects.find(name);

    if (it != m_objects.end()) {
        Commandable *receiver = it->second;

        CommandInput comInput;
        comInput.m_keyword     = command;
        comInput.m_code        = receiver->translate(command);
        comInput.m_arguments   = tokens;
        std::cout << "Got " << comInput.m_keyword << " " << comInput.m_code << "\n";

        // First try the built-in commands.
        result = receiver->receiveBuiltInCommand(comInput);
        // Then try the objects self-defined commands.
        if (result.m_code == CommandResult::ResultCode::NO_RECEIVER) {
            std::cout << comInput.m_keyword << " is not builtin...\n";
            result = receiver->receiveCommand(comInput);
        }
    }
    else {
        result.m_meta = std::string("Couldn't find a receiver object of the name ") + name; 
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

    // Insert object into type listing.
    ObjectTypeNameToVectorType::iterator nameIt = m_typelist.find(type);
    if (nameIt == m_typelist.end()) {
        m_typelist[type] = std::vector<Commandable*>();
    }
    m_typelist[type].push_back(object);

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
