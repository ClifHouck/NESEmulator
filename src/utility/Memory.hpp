#ifndef MEMORY_H
#define MEMORY_H

#include "DataTypes.hpp"

#include <string>
#include <vector>

class Memory
{
public:
    typedef unsigned int size_t;
    typedef u16_word     address_t;
    typedef u8_byte      data_t;

    Memory(size_t size);
    Memory(address_t startAddress,
           address_t endAddress);
    virtual ~Memory();

    address_t   startAddress() const;
    address_t   endAddress()   const;

    virtual void setAddressRange(address_t start, address_t end);

    // Checked reads and writes with debugging if needed.
    u8_byte     read(const address_t address);
    void        write(const address_t address, const data_t data);

    // Raw read/writes aren't checked in any appreciable way.
    void        rawWrite(const address_t address, const data_t data);

    u8_byte rawReadByte(const address_t address) {
        return getData(address);
    }

    u16_word rawReadWord(const address_t address) {
        return  static_cast<u16_word>(getData(address + 1)) * 0x100 + 
                static_cast<u16_word>(getData(address));
    }

    size_t size() const;

    virtual Memory* clone() = 0;

protected:
    virtual data_t  getData(address_t address) = 0;
    virtual void    setData(address_t address, data_t data) = 0;

    address_t   m_startAddress;
    address_t   m_endAddress;
    size_t      m_size;
};

class BackedMemory : public Memory
{
public:
    BackedMemory(address_t beginAddress, address_t endAddress);
    BackedMemory(size_t size);
    BackedMemory(size_t size, data_t *initData);

    BackedMemory(const BackedMemory& other);
    BackedMemory& operator=(BackedMemory tmp);

    virtual ~BackedMemory();

    virtual void setAddressRange(address_t begin, address_t end);

    virtual Memory* clone();

protected:
    virtual data_t  getData(address_t address);
    virtual void    setData(address_t address, data_t data);

    address_t correctedAddress(address_t address) const;

private:
    u8_byte *m_backing;
};

class MappedMemory : public Memory
{
public:
    MappedMemory(address_t startAddress, 
                 address_t endAddress,
                 std::vector<Memory*> segments);

    MappedMemory(const MappedMemory& other);
    MappedMemory& operator=(MappedMemory tmp);

    virtual ~MappedMemory() {}

    void addSegment(Memory * segment);
    void removeSegment(address_t address);

    virtual Memory* clone();

    std::string debugInfo() const;
    std::string segmentInfo(Memory * segment) const;

protected:
    virtual data_t  getData(address_t address);
    virtual void    setData(address_t address, data_t data);

    Memory*                         findMemorySegment(address_t address);
    std::vector<Memory*>::iterator  findMemorySegmentIterator(address_t address);

    std::vector<Memory*> m_segments;
};

#endif //MEMORY_H
