#include "Memory.hpp"

#include <cassert>

Memory::
Memory(size_t size) :
    m_size (size),
    m_backing (nullptr)
{
    m_backing = new u8_byte[size];
}

Memory::
~Memory()
{
    delete[] m_backing;
    m_backing = nullptr;
}

u8_byte 
Memory::
read(const address_t address) const
{
    assert(address > 0);
    assert(address < m_size);
    return m_backing[address];
}

void    
Memory::
write(const address_t address, data_t data)
{
    assert(address > 0);
    assert(address < m_size);
    m_backing[address] = data;
}
