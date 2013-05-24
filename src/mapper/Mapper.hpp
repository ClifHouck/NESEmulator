#ifndef NES_MAPPER_H
#define NES_MAPPER_H

class Mapper 
{
public:
    Mapper();

    //48k - roughly the size of the cartridge reserved area in the CPU memory map.
    static const unsigned int CartdrigeSize = 48 * 1024; 

    unsigned int iNESNumber() const;

private:
    unsigned int    m_iNESNumber;
};

#endif //NES_MAPPER_H
