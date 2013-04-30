#ifndef MEMORY_H
#define MEMORY_H

#include "DataTypes.hpp"

class Memory
{
public:
    typedef unsigned int size_t;
    typedef unsigned int address_t;
    typedef u8_byte      data_t;

    Memory(size_t size);
    ~Memory();

    u8_byte     read(const address_t address);
    void        write(const address_t address, const data_t data);

    template <typename ReturnType>  
    ReturnType rawRead(const address_t address);

    template <u8_byte> u8_byte rawRead(const address_t address) {
        return getData(address);
    }

    template <u16_word> u16_word rawRead(const address_t address) {
        return  static_cast<u16_word>(getData(address)) * 0x100 + 
                static_cast<u16_word>(getData(address + 1));
    }

    size_t size() const;

protected:
    virtual data_t  getData(address_t address);
    virtual void    setData(address_t address, data_t data);

private:
    u8_byte *m_backing;
    size_t   m_size;
};

#endif //MEMORY_H
