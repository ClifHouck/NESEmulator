/*
   This is derived mainly from the description found here:
   http://nocash.emubase.de/everynes.htm#cpu65xxmicroprocessor
*/

#ifndef CPU65XX_H
#define CPU65XX_H

#include "DataTypes.hpp"

#include <bitset>
#include <string>
#include <functional>
#include <map>
#include <set>

#define MAIN_MEM_SIZE (64 * 1024)

class Cpu65XX
{
    public:
        Cpu65XX();
        Cpu65XX(u8_byte * toLoad, unsigned int size);

        ~Cpu65XX();

        class StatusRegister {
            public:
                StatusRegister();
                StatusRegister(u8_byte&);

                // accessors
                bool carry()        const;
                bool zero()         const;
                bool IRQDisable()   const;
                bool decimalMode()  const;
                bool breakFlag()    const;
                bool overflow()     const;
                bool negative()     const;
                bool unusedFlag()   const;

                u8_byte value() const;

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
                Memory(u8_byte * toLoad, unsigned int size);
                
                // mutators
                u8_byte&  byteAt(const u16_word& address);
                u8_byte&  byteAtZeroPage(const u8_byte& address);

                // accessors
                u16_word  wordAt(const u16_word& address, bool indirect = false);
                u16_word  wordAtZeroPage(const u8_byte& address);

            private:
                u16_word trueAddress(const u16_word&) const;

                u8_byte m_memory[MAIN_MEM_SIZE];
        }; 

        class Instruction {
            public:
                Instruction();
                Instruction(
                        u8_byte opcode,
                        u8_byte length,
                        const char* mnemonic,
                        std::function<std::string()>& dissasemblyFunc,
                        std::function<unsigned int()>& cycleFunc,
                        std::function<void ()>& workFunc
                );

                const u8_byte&      opcode()        const;
                const u8_byte&      length()        const;
                unsigned int        cycles()        const;
                const std::string&  mnemonic()      const;
                std::string         disassembly()   const;
                void                apply()         const;

            private:
                u8_byte         m_opcode;
                u8_byte         m_length;
                std::function<unsigned int()> m_cycleFunction;
                std::function<void ()>        m_workFunction;
                std::function<std::string ()> m_disassemblyFunction;
                std::string     m_mnemonic;
        };

        void tick();
        void signalNMI();

        // accessors
        const u8_byte&           A()  const;
        const u8_byte&           X()  const;
        const u8_byte&           Y()  const;
        const u16_word&          PC() const;
        const u8_byte&           S()  const;
        u16_word                 stackPointer() const;
        const StatusRegister&    statusRegister() const;


        bool                     crossesPageBoundary(const u16_word& address) const;
        bool                     crossesPageBoundary(const u16_word& address, const u8_byte& offset) const;
        bool                     conditionalBranchCrossesPageBoundary(const u8_byte& offset) const;

        std::string state() const;
        std::string statusRegisterState() const;
        std::string debugOutput();

        // mutators
        void    setA(const u8_byte&);
        void    setX(const u8_byte&);
        void    setY(const u8_byte&);
        void    setPC(const u16_word&);
        void    setS(const u8_byte&);
        void    setStatusRegister(const StatusRegister&);

        u8_byte&  byteOperand();
        u16_word  wordOperand();

        void    handleRegisterAssignmentFlags(const u8_byte&);

        u8_byte additionWithCarry(const u8_byte&, const u8_byte&);
        u8_byte subtractionWithBorrow(const u8_byte&, const u8_byte&);
        void    compare(const u8_byte&, const u8_byte&);

        u8_byte shiftLeft(const u8_byte&);
        u8_byte shiftRight(const u8_byte&);

        u8_byte rotateLeftThroughCarry(const u8_byte&);
        u8_byte rotateRightThroughCarry(const u8_byte&);

        void setByte(u8_byte&, const u8_byte&);

        void bitTest(const u8_byte&, const u8_byte&);

        void conditionalBranch(bool condition, const u8_byte&);

        // Stack operations
        void pushStackByte(const u8_byte&);
        void pushStackWord(const u16_word&);
        u8_byte&   popStackByte();
        u16_word   popStackWord();

        Memory&     memory();

    private:
        void buildInstructionSet();
        void buildDisassemblyFunctions();

        // Registers
        u8_byte           m_A;  // Accumulator
        u8_byte           m_X;
        u8_byte           m_Y;
        u16_word          m_PC; // Program counter
        u8_byte           m_S;  // Stack pointer
        StatusRegister    m_status;

        Memory            m_memory;

        //Set of instructions used by the CPU
        Instruction*      m_instructions;
        std::map<const char*, std::function<std::string ()> > m_disassemblyFunctions;
        std::set<u8_byte> m_illegalInstructions;
        // Cycles to wait until executing the current instruction.
        unsigned int      m_downCycles; 
        // The next instruction to run.
        Instruction*      m_queuedInstruction;

        // Has an NMI been requested? (This is likely the screen redraw NMI)
        bool              m_NMI;
        // IRQs requested?
        bool              m_IRQ;

        unsigned int      m_cycles;
};

#endif 
