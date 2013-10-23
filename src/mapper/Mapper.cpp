#include "Mapper.hpp"

#include "MMC1Mapper.hpp"

#include <cassert>

Mapper::
Mapper()
{}

Mapper::
~Mapper()
{}

Mapper*
Mapper::
getMapper(iNESFile &file)
{
    switch (file.mapperNumber()) {
        case MMC1:
            return new MMC1Mapper(file);
        default:
            assert(false && "Unhandled mapper encountered!");
    }
    return nullptr;
}
