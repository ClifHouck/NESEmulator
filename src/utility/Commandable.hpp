#ifndef COMMANDABLE_H
#define COMMANDABLE_H

#include <string>
#include <vector>
#include <map>

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
    int         m_numArguments;
};

class Commandable
{
public:
    Commandable(std::string name);

    virtual std::vector<Command> commands() = 0;
    virtual CommandResult        recieveCommand(const std::string& input) = 0;
    std::string name() const;
    void        setName(std::string name);

private:
    std::string m_name;
};

class CommandDispatcher
{
public:
    CommandResult command(const std::string& input);
    bool          registerObject(Commandable* object);
    bool          removeObject(Commandable* object);

private:
    typedef std::map<std::string, Commandable*> MapType;
    MapType m_objects;
};

#endif //COMMANDABLE_H
