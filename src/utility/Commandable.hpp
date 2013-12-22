#ifndef COMMANDABLE_H
#define COMMANDABLE_H

#include <string>
#include <vector>
#include <map>

#include <limits.h>

typedef unsigned int CommandCode;
const CommandCode UNRECOGNIZED_COMMAND = UINT_MAX;
const CommandCode HELP_COMMAND         = UINT_MAX - 1;

struct CommandResult
{
    CommandResult() :
        m_output (),
        m_meta (),
        m_code (NO_RECEIVER),
        m_data (nullptr)
    {}

    enum ResultCode {   
            OK, 
            WARNING, 
            ERROR, 
            INVALID_ARGUMENT, 
            WRONG_NUM_ARGS, 
            NO_RECEIVER,
    }; 

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

    virtual CommandResult        receiveCommand(CommandInput command) = 0;
    virtual std::string          typeName() = 0;

    CommandMapType  commands();
    CommandCode translate(std::string keyword);
    CommandResult receiveBuiltInCommand(CommandInput command);
    std::string name() const;

protected:
    void setName(std::string name);
    void addCommand(Command command);
    std::string help();

private:
    CommandMapType      m_commands;
    TranslationMapType  m_translation;
    std::string         m_name;
};

class CommandDispatcher
{
public:
    CommandResult command(const std::string& input);

    // This waits to register the object until the next time
    // the dispatcher receives a command or the dispatcher.
    // is explicitly told to process delayed registration objects.
    void          delayedRegister(Commandable* object);
    void          finishRegistration();

    bool          registerObject(Commandable* object);
    bool          removeObject(Commandable* object);

    static CommandDispatcher* instance();

private:
    std::vector<std::string> parseArguments(std::string input);
    CommandResult handleBuiltInCommand(std::string input);

    typedef std::map<std::string, Commandable*> ObjectMapType;
    // Maps types to their command maps...
    typedef std::map<std::string, std::map<CommandCode, Command>> CommandMapType;
    CommandMapType            m_commands;
    ObjectMapType             m_objects;
    std::vector<Commandable*> m_awaitingRegistration;
};

#endif //COMMANDABLE_H
