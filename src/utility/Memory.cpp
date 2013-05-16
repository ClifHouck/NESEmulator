#include "Memory.hpp"

#include <cassert>
#include <algorithm>

Memory::
Memory(size_t size) :
    m_size (size),
    m_backing (nullptr)
{
    m_backing = new data_t[size];
    assert(m_backing != nullptr);
}

Memory::
Memory(size_t size,
       u8_byte *initData) :
    m_size (size),
    m_backing (nullptr)
{
    m_backing = new data_t[size];
    assert(m_backing != nullptr);
    std::copy(initData, initData + size, m_backing);
}

Memory::
~Memory()
{
    delete[] m_backing;
    m_backing = nullptr;
}

u8_byte 
Memory::
read(const address_t address) 
{
    return getData(address);
}

void    
Memory::
write(const address_t address, data_t data)
{
    assert(address > 0);
    assert(address < m_size);
    setData(address, data);
}

Memory::data_t
Memory::
getData(address_t address)
{
    assert(address > 0);
    assert(address < m_size);
    return m_backing[address];
}

void
Memory::
setData(address_t address, data_t data)
{
    assert(address > 0);
    assert(address < m_size);
    m_backing[address] = data;
}

void        
Memory::
rawWrite(const address_t address, const data_t data)
{
    setData(address, data);
}
