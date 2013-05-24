#include "Mapper.hpp"

Mapper::
Mapper() :
    m_iNESNumber(0)
{}

unsigned int
Mapper::
iNESNumber() const
{
    return m_iNESNumber;
}
