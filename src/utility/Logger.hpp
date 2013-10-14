#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>

class Logger
{
public:
    Logger() : m_outstream (&std::cout)
    {}
    Logger(std::ostream &outstream) : 
        m_outstream (&outstream) 
    {}

    void setOutStream(std::ostream &outstream) {
        m_outstream = &outstream;
    }

    template <typename T>
    std::ostream& operator<<(T val) { return *m_outstream << val; }   

    static Logger * get_instance() {
        static Logger logger;
        return &logger;
    }

private:
    std::ostream *m_outstream;
};

#endif //LOGGER_H
