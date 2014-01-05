#include "Mapper.hpp"

#include "NROMMapper.hpp"
#include "MMC1Mapper.hpp"

#include <iostream>
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
        case NROM:
            return new NROMMapper(file);
        case MMC1:
            return new MMC1Mapper(file);
        default:
        {
            // FIXME: Send to logger instead?
            std::cerr << "Unhandled mapper number encountered! Number: " << file.mapperNumber() << std::endl;
            assert(false && "Unhandled mapper encountered!");
        }
    }
    return nullptr;
}
