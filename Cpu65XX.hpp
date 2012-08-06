/*
   This is derived mainly from the description found here:
   http://nocash.emubase.de/everynes.htm#cpu65xxmicroprocessor
*/

#include <bitset>
#include <string>

#define MAIN_MEM_SIZE 64 * 1024

class Cpu65XX
{
    public:
        Cpu65XX();
        Cpu65XX(unsigned char * toLoad, unsigned int size);

        class StatusRegister {
            public:
                StatusRegister();
                StatusRegister(unsigned char&);

                // accessors
                bool carry()        const;
                bool zero()         const;
                bool IRQDisable()   const;
                bool decimalMode()  const;
                bool breakFlag()    const;
                bool overflow()     const;
                bool negative()     const;

                unsigned char value() const;

                // mutators
                void setCarry(bool);
                void setZero(bool);
                void setIRQDisable(bool);
                void setDecimalMode(bool);
                void setBreakFlag(bool);
                void setOverflow(bool);
                void setNegative(bool);

            private:
                // Status Register
                std::bitset<8> m_status;
        };

        class Memory {
            public: 
                Memory();
                Memory(unsigned char * toLoad, unsigned int size);
                
                // mutators
                unsigned char&  byteAt(const unsigned short&);
                unsigned short& wordAt(const unsigned short&);

            private:
                unsigned char m_memory[MAIN_MEM_SIZE];
        };

        void execute();

        // accessors
        const unsigned char&           A() const;
        const unsigned char&           X() const;
        const unsigned char&           Y() const;
        const unsigned short&          PC() const;
        const unsigned char&           S() const;
        unsigned short                 stackPointer() const;
        const StatusRegister& statusRegister() const;

        std::string state() const;

        // mutators
        void    setA(const unsigned char&);
        void    setX(const unsigned char&);
        void    setY(const unsigned char&);
        void    setPC(const unsigned short&);
        void    setS(const unsigned char&);
        void    setStatusRegister(const StatusRegister&);

        void    handleRegisterAssignmentFlags(const unsigned char&);

        unsigned char additionWithCarry(const unsigned char&, const unsigned char&);
        unsigned char subtractionWithBorrow(const unsigned char&, const unsigned char&);
        void          compare(const unsigned char&, const unsigned char&);

        unsigned char shiftLeft(const unsigned char&);
        unsigned char shiftRight(const unsigned char&);

        unsigned char rotateLeftThroughCarry(const unsigned char&);
        unsigned char rotateRightThroughCarry(const unsigned char&);

        void setByte(unsigned char&, const unsigned char&);

        void bitTest(const unsigned char&, const unsigned char&);

        void conditionalBranch(bool condition, const unsigned char&);

        void pushStackByte(const unsigned char&);
        void pushStackWord(const unsigned short&);

        unsigned char&   popStackByte();
        unsigned short&  popStackWord();

    private:
        // Registers
        unsigned char           m_A;  // Accumulator
        unsigned char           m_X;
        unsigned char           m_Y;
        unsigned short          m_PC; // Program counter
        unsigned char           m_S;  // Stack pointer
        StatusRegister m_status;
        Memory         m_memory;
};
