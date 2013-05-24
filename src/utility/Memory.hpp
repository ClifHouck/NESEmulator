#ifndef MEMORY_H
#define MEMORY_H

#include "DataTypes.hpp"

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
    virtual ~BackedMemory();

protected:
    virtual data_t  getData(address_t address);
    virtual void    setData(address_t address, data_t data);

private:
    u8_byte *m_backing;
};

class MappedMemory : public Memory
{
public:
    MappedMemory(address_t startAddress, 
                 address_t endAddress,
                 std::vector<Memory*> segments);

protected:
    virtual data_t  getData(address_t address);
    virtual void    setData(address_t address, data_t data);

    Memory*  findMemorySegment(address_t address);

    std::vector<Memory*> m_segments;
};

#endif //MEMORY_H
