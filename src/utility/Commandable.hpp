#ifndef COMMANDABLE_H
#define COMMANDABLE_H

#include <string>
#include <vector>
#include <map>

#include <limits.h>

typedef unsigned int CommandCode;
const CommandCode UNRECOGNIZED_COMMAND = UINT_MAX;

struct CommandResult
{
    enum ResultCode { OK, WARNING, ERROR, INVALID_ARGUMENT, WRONG_NUM_ARGS, NO_RECIEVER }; 

    std::string m_output;
    std::string m_meta;   // Output /about/ the result of the command.
    ResultCode  m_code;
    void*       m_data;
};

struct Command
{
    std::string m_keyword;
    std::string m_helpText;
    CommandCode m_code;
    int         m_numArguments;
};

struct CommandInput
{
    std::string              m_keyword;
    CommandCode              m_code;
    std::vector<std::string> m_arguments;
};

class Commandable
{
public:
    Commandable(std::string name);

    typedef std::map<CommandCode, Command>      CommandMapType;
    typedef std::map<std::string, CommandCode>  TranslationMapType;

    virtual CommandResult        recieveCommand(CommandInput command) = 0;
    virtual std::string          typeName() = 0;

    CommandMapType  commands();
    CommandCode translate(std::string keyword);
    std::string name() const;

protected:
    void setName(std::string name);
    void addCommand(Command command);

private:
    CommandMapType      m_commands;
    TranslationMapType  m_translation;
    std::string         m_name;
};

class CommandDispatcher
{
public:
    CommandResult command(const std::string& input);
    bool          registerObject(Commandable* object);
    bool          removeObject(Commandable* object);

private:
    std::vector<std::string> parseArguments(std::string input);

    typedef std::map<std::string, Commandable*> ObjectMapType;
    // Maps types to their command maps...
    typedef std::map<std::string, std::map<CommandCode, Command>> CommandMapType;
    CommandMapType m_commands;
    ObjectMapType  m_objects;
};

#endif //COMMANDABLE_H
