#include "iNESFile.hpp"

#include <sstream>
#include <iostream>
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
    m_inFile     (filename, std::ios::in | std::ios::binary),
    m_fileData   (0)
{
    load();
}

void 
iNESFile::
load()
{
    if (m_fileData) {
        delete[] m_fileData;
        m_fileData = 0;
    }

    if (m_inFile.is_open() &&
        m_inFile.good()) {

        unsigned int begin = m_inFile.tellg();
        m_inFile.seekg(0, std::ios::end);
        unsigned int end = m_inFile.tellg();
        m_fileSize = end - begin;

        m_inFile.seekg(0, std::ios::beg);

        // Load the data from the file.
        // m_fileSize = m_inFile.tellg();
        // Bail out if the file is too small.
        if (m_fileSize < 16) {
            return;
        }

        m_fileData = new char[m_fileSize];
        m_inFile.read(m_fileData, m_fileSize);
        m_inFile.close();

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
iNESFile::
PRGROMDataSize() const
{
    return m_PRGROMSize * 16 * 1024;
}

unsigned int 
iNESFile::
CHRROMDataSize() const
{
    return m_CHRROMSize * 8 * 1024;
}

// Flags 6
unsigned int 
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
    if (m_flags7 & 0x0C == 0x08) {
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
