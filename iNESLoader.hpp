/*
   Derived from http://wiki.nesdev.com/w/index.php/INES
*/

#include <string>
#include <ifstream>

class iNESFile 
{
    public:
        iNESFile(const char* filename);

        void load();
        bool isGood() const;

        bool isiNESFile()                   const;
        
        unsigned int PRGROMDataSize()       const;
        unsigned int CHRROMDataSize()       const;

        // Flags 6
        unsigned int mirrorMode()           const;
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

    private:
        u8_byte m_banner[4];
        u8_byte m_PRGROMSize;
        u8_byte m_CHRROMSize;
        u8_byte m_flags6;
        u8_byte m_flags7;
        u8_byte m_PRGRAMSize;
        u8_byte m_flags9;
        u8_byte m_flags10;

        std::string filename;
        ifstream    m_inFile;
        char *      m_fileData;
};
