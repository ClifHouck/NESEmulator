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

    virtual u8_byte     read(const address_t address) const;
    virtual void        write(const address_t address, const data_t data);

    template <typename ReturnType>  
    ReturnType rawRead(const address_t address) const;

    template <u8_byte> u8_byte rawRead(const address_t address) const {
        return m_backing[address];
    }

    template <u16_word> u16_word rawRead(const address_t address) const {
        return *(static_cast<u16_word*>(static_cast<void*>(m_backing[address])));
    }

    size_t size() const;

protected:
    virtual void addressMap() const;

private:
    u8_byte *m_backing;
    size_t   m_size;

};

#endif //MEMORY_H
