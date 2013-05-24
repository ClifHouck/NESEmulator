#include "Memory.hpp"

#include <cassert>
#include <algorithm>

Memory::
Memory(address_t startAddress,
       address_t endAddress) :
    m_startAddress(startAddress),
    m_endAddress(endAddress)
{
    assert(startAddress < endAddress);
    m_size = endAddress - startAddress;
    assert(m_size > 0);
}

Memory::
Memory(size_t size) :
    m_size (size)
{
    assert(size > 0);
    m_startAddress = 0;
    m_endAddress = m_size - 1;
}

Memory::
~Memory()
{
}

Memory::address_t
Memory::
startAddress() const
{
    return m_startAddress;
}

Memory::address_t
Memory::
endAddress() const
{
    return m_endAddress;
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
    setData(address, data);
}

void        
Memory::
rawWrite(const address_t address, const data_t data)
{
    setData(address, data);
}

BackedMemory::
BackedMemory(address_t beginAddress, address_t endAddress) :
    Memory(beginAddress, endAddress)
{
    m_backing = new data_t[m_size];
    assert(m_backing != nullptr);
}

BackedMemory::
~BackedMemory()
{
    delete[] m_backing;
    m_backing = nullptr;
}

BackedMemory::
BackedMemory(size_t size) :
    Memory(size)
{
    m_backing = new data_t[size];
    assert(m_backing != nullptr);
}

BackedMemory::
BackedMemory(size_t size, 
        data_t *initData) :
    Memory(size)
{
    m_backing = new data_t[size];
    assert(m_backing != nullptr);
    std::copy(initData, initData + size, m_backing);
}

Memory::data_t
BackedMemory::
getData(address_t address)
{
    assert(address >= m_startAddress); 
    assert(address <= m_endAddress);
    assert(address >= 0);
    assert(address < m_size);
    return m_backing[address];
}

void
BackedMemory::
setData(address_t address, data_t data)
{
    assert(address >= m_startAddress); 
    assert(address <= m_endAddress);
    assert(address >= 0);
    assert(address < m_size);
    m_backing[address] = data;
}

MappedMemory::
MappedMemory(address_t startAddress, 
             address_t endAddress,
             std::vector<Memory*> segments) :
    Memory(startAddress, endAddress),
    m_segments (segments)
{
    assert(segments.size() > 0);
}

Memory::data_t  
MappedMemory::
getData(address_t address)
{
    Memory *segment = findMemorySegment(address);
    assert(segment != nullptr);
    return segment->read(address);
}

void    
MappedMemory::
setData(address_t address, data_t data)
{
    Memory *segment = findMemorySegment(address);
    assert(segment != nullptr);
    segment->write(address, data);
}

Memory*
MappedMemory::
findMemorySegment(address_t address)
{
    Memory *foundSegment = nullptr;

    //TODO: More efficent memory segment find. This is O(n).
    for (auto iter = m_segments.begin(); iter != m_segments.end(); ++iter) {
        Memory *segment = *iter;
        if (address >= segment->startAddress() &&
            address <= segment->endAddress()) {
            foundSegment = segment;
            break;
        }
    }

    return foundSegment;
}
