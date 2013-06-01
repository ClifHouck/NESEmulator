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

void
Memory::
setAddressRange(address_t begin, address_t end)
{
    assert(begin < end);
    assert((end - begin) > 0);

    m_startAddress = begin;
    m_endAddress = end;
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

void
BackedMemory::
setAddressRange(address_t begin, address_t end)
{
    Memory::setAddressRange(begin, end);

    assert((begin - end) == m_size && 
           "BackedMemory does not support resizing the address range!");
}

Memory::address_t
BackedMemory::
correctedAddress(address_t address) const
{
    return address - m_startAddress;
}

Memory::data_t
BackedMemory::
getData(address_t address)
{
    assert(address >= m_startAddress); 
    assert(address <= m_endAddress);

    return m_backing[correctedAddress(address)];
}

void
BackedMemory::
setData(address_t address, data_t data)
{
    assert(address >= m_startAddress); 
    assert(address <= m_endAddress);
    m_backing[correctedAddress(address)] = data;
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

void
MappedMemory::
addSegment(Memory *segment)
{
    assert(segment != nullptr);
    m_segments.push_back(segment);
}

void
MappedMemory::
removeSegment(address_t address) 
{
    Memory *segment = findMemorySegment(address);
}

Memory::data_t  
MappedMemory::
getData(address_t address)
{
    Memory *segment = findMemorySegment(address);
    return segment->read(address);
}

void    
MappedMemory::
setData(address_t address, data_t data)
{
    Memory *segment = findMemorySegment(address);
    segment->write(address, data);
}

std::vector<Memory*>::iterator
MappedMemory::
findMemorySegmentIterator(address_t address)
{
    std::vector<Memory*>::iterator iter = m_segments.end();

    //TODO: More efficent memory segment find. This is O(n).
    for (iter = m_segments.begin(); iter != m_segments.end(); ++iter) {
        Memory *segment = *iter;
        if (address >= segment->startAddress() &&
            address <= segment->endAddress()) {
            break;
        }
    }

    //TODO : Default behavior and warnings when it can't find the memory segment to match an
    // address?
    assert(iter != m_segments.end());

    return iter;
}

Memory*
MappedMemory::
findMemorySegment(address_t address)
{
    auto iter = findMemorySegmentIterator(address);
    return *iter;
}
