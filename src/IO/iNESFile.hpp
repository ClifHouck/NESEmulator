/*
   Derived from http://wiki.nesdev.com/w/index.php/INES
*/

#include "utility/DataTypes.hpp"

#include <string>
#include <fstream>

class iNESFile 
{
    public:
        enum MirrorType {
            horizontalMirroring = 0,
            verticalMirroring   = 1,
            fourScreenMirroring = 2
        };

        static const unsigned int PRG_ROM_PAGE_SIZE = 16 * 1024;
        static const unsigned int CHR_ROM_PAGE_SIZE = 8  * 1024;

        iNESFile(const char* filename);

        void load();
        bool isGood() const;

        std::string headerInfo() const;

        const std::string& filename() const;

        bool isiNESFile()                   const;
        
        unsigned int numberOfPRGROMPages()  const;
        unsigned int PRGROMDataSize()       const;
        unsigned int numberOfCHRROMPages()  const;
        unsigned int CHRROMDataSize()       const;

        // Flags 6
        MirrorType   mirrorMode()           const;
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

        // Flags 10
        // TODO...
        char *          m_fileData;

    private:
        u8_byte m_banner[4];
        u8_byte m_PRGROMSize;
        u8_byte m_CHRROMSize;
        u8_byte m_flags6;
        u8_byte m_flags7;
        u8_byte m_PRGRAMSize;
        u8_byte m_flags9;
        u8_byte m_flags10;

        std::string     m_filename;
        std::ifstream   m_inFile;
        unsigned int    m_fileSize;
};
