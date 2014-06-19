#include "iNESFile.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

iNESFile::
iNESFile(const char* filename) :
    m_PRGROMSize (0),
    m_CHRROMSize (0),
    m_flags6     (0),
    m_flags7     (0),
    m_PRGRAMSize (0),
    m_flags9     (0),
    m_flags10     (0),
    m_filename   (filename),
    m_fileData   (nullptr)
{
    load();
}

void 
iNESFile::
load()
{
    if (m_fileData != nullptr) {
        delete[] m_fileData;
        m_fileData = nullptr;
    }

    std::ifstream inFile(m_filename, std::ios::in | std::ios::binary);

    if (inFile.is_open() &&
        inFile.good()) {

        unsigned int begin = inFile.tellg();
        inFile.seekg(0, std::ios::end);
        unsigned int end = inFile.tellg();
        m_fileSize = end - begin;

        inFile.seekg(0, std::ios::beg);

        // Load the data from the file.
        // m_fileSize = inFile.tellg();
        // Bail out if the file is too small.
        if (m_fileSize < HEADER_SIZE) {
            // FIXME: Return error.
            return;
        }

        m_fileData = new char[m_fileSize];
        inFile.read(m_fileData, m_fileSize);
        inFile.close();

        // Fill out the header elements;
        std::copy(m_fileData, m_fileData + 4, m_banner);
        m_PRGROMSize = m_fileData[4];
        m_CHRROMSize = m_fileData[5];
        m_flags6     = m_fileData[6];
        m_flags7     = m_fileData[7];
        m_PRGRAMSize = m_fileData[8];
        m_flags9     = m_fileData[9];
        m_flags10    = m_fileData[10];
    }
    // TODO: Handle file read failure.
}

std::string 
iNESFile::
headerInfo() const
{
    std::stringstream output;
    output << "Is iNES file?            " << isiNESFile()           << std::endl
           << "iNES header version:     " << headerVersion()        << std::endl
           << "PRG ROM Data Size:       " << PRGROMDataSize()       << std::endl
           << "CHR ROM Data Size:       " << CHRROMDataSize()       << std::endl
           << "Mirroring mode:          " << mirrorMode()           << std::endl
           << "Battery backed SRM?      " << batteryBackedSRAM()    << std::endl
           << "Trainer present?         " << trainerPresent()       << std::endl
           << "VS Unisystem?            " << VSUnisystem()          << std::endl
           << "PlayChoice 10?           " << PlayChoice10()         << std::endl
           << "Mapper number            " << mapperNumber()         << std::endl;
    return output.str();
}

const std::string& 
iNESFile::
filename() const
{
    return m_filename;
}

bool 
iNESFile::
isiNESFile() const
{
    return  m_banner[0] == 'N' &&
            m_banner[1] == 'E' &&
            m_banner[2] == 'S' &&
            m_banner[3] == 0x1A; 
}

unsigned int 
iNESFile::numberOfPRGROMPages() const
{
    return m_PRGROMSize;
}
        
unsigned int 
iNESFile::
PRGROMDataSize() const
{
    return m_PRGROMSize * PRG_ROM_PAGE_SIZE;
}

unsigned int 
iNESFile::numberOfCHRROMPages() const
{
    return m_CHRROMSize;
}

unsigned int 
iNESFile::
CHRROMDataSize() const
{
    return m_CHRROMSize * CHR_ROM_PAGE_SIZE;
}

// Flags 6
iNESFile::MirrorType
iNESFile::
mirrorMode() const
{
    u8_byte mirroringMode = m_flags6 & 0x09;
    if (mirroringMode == 0x00) {
        return horizontalMirroring;
    }
    else if (mirroringMode == 0x01) {
        return verticalMirroring;
    }
    return fourScreenMirroring;
}

bool         
iNESFile::
batteryBackedSRAM() const
{
    return (m_flags6 & 0x02);
}

bool         
iNESFile::
trainerPresent() const
{
    return (m_flags6 & 0x04);
}

// Flags 7
bool         
iNESFile::
VSUnisystem() const
{
    return (m_flags7 & 0x01);
}

bool         
iNESFile::
PlayChoice10() const
{
    return (m_flags7 & 0x02);
}

unsigned int 
iNESFile::
headerVersion() const
{
    if ((m_flags7 & 0x0C) == 0x08) {
        return 2;
    }
    return 1;
}

// Flags 6&7
unsigned int 
iNESFile::
mapperNumber() const
{
    return (m_flags7 & 0xF0) & ((m_flags6 & 0xF0) >> 4);
}

// Flags 9
bool         
iNESFile::
NTSC() const
{
    return (m_flags9 & 0x01) == 0;
}

bool         
iNESFile::
PAL() const
{
    return (m_flags9 & 0x01) != 0;
}

u8_byte*
iNESFile::
prgRomPage(unsigned int n)
{
    assert(n < numberOfPRGROMPages() && "Invalid PRG ROM page requested");
    unsigned int offset = HEADER_SIZE + (trainerPresent() * TRAINER_SIZE) + (PRG_ROM_PAGE_SIZE * n);
    return reinterpret_cast<u8_byte*>(&(m_fileData[offset]));
}

u8_byte*
iNESFile::
vromPage(unsigned int n)
{
    assert(n < numberOfCHRROMPages() && "Invalid CHR ROM page requested");
    unsigned int offset = HEADER_SIZE + (trainerPresent() * TRAINER_SIZE) + (PRGROMDataSize()) + (CHR_ROM_PAGE_SIZE * n);
    return reinterpret_cast<u8_byte*>(&(m_fileData[offset]));
}
