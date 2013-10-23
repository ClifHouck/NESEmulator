#ifndef NES_MAPPER_H
#define NES_MAPPER_H

#include "utility/Memory.hpp"
#include "IO/iNESFile.hpp"

class Mapper 
{
public:
    Mapper();
    virtual ~Mapper();

    enum MapperNumber {
        NROM = 0,
        MMC1 = 1,
        UNROM = 2,
        CNROM = 3
    };

    virtual Memory *cpuMemory() = 0;
    virtual Memory *ppuMemory() = 0;

    virtual const char* name() const = 0;

    //Constructs and returns an appropriate Mapper for the supplied
    //iNESFile argument.
    //TODO: Used some sort of shared_ptr instead?
    static Mapper* getMapper(iNESFile &file);
};

#endif //NES_MAPPER_H
