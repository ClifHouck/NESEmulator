#include "iNESFile.hpp"

iNESFile::
iNESFile(const char* filename) :
    m_PRGROMSize (0),
    m_CHRROMSize (0),
    m_flags6     (0),
    m_flags7     (0),
    m_PRGRAMSize (0),
    m_flags9     (0),
    m_flag10     (0)
    m_filename   (filename),
    m_inFile     (m_filename, std::ios::in | std::ios::binary),
    m_fileData   (0)
{
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

        // Load the data from the file.
        m_fileSize = inFile.tellg();
        // Bail out if the file is too small.
        if (m_fileSize < 32) {
            return;
        }

        m_fileData = new char[fileSize];
        m_inFile.read(fileData, fileSize);
        m_inFile.close();

        // Fill out the header elements;
        std::copy(fileData, fileData + 4, m_banner);
        m_PRGROMSize = m_fileData[4];
        m_CHRROMSize = m_fileData[5];
        m_flags6     = m_fileData[6];
        m_flags7     = m_fileData[7];
        m_PRGRAMSize = m_fileData[8];
        m_flags9     = m_fileData[9];
        m_flags10    = m_fileData[10];
    }
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
    if ((m_flags6 & 0x11) == 0x00) {
        return horizontalMirroring;
    }
}
        bool         batteryBackedSRAM()    const;
        bool         trainerPresent()       const;

        // Flags 7
        bool         VSUnisystem()          const;
        bool         PlayChoice10()         const;
        unsigned int headerVersion()        const;

        // Flags 6&7
        unsigned int mapperNumber()         const;

        // Flags 9
        bool         NTSC()                 const;
        bool         PAL()                  const;


