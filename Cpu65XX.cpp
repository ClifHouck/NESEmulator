#include "Cpu65XX.hpp"

#include <iostream>
#include <sstream>

Cpu65XX::
Cpu65XX() :
    m_A (0),
    m_X (0),
    m_Y (0),
    m_PC (0),
    m_S (0xFF)
{
}

Cpu65XX::
Cpu65XX(unsigned char * toLoad, unsigned int size) :
        m_memory (toLoad, size), 
        m_A (0),
        m_X (0),
        m_Y (0),
        m_PC (0),
        m_S (0xFF)
{
}

void
Cpu65XX::
execute()
{
    const unsigned char&  instruction = m_memory.byteAt(PC());
    const unsigned char&  operand1    = m_memory.byteAt(1 + PC());
    const unsigned char&  operand2    = m_memory.byteAt(2 + PC());
    const unsigned short& wordOperand = m_memory.wordAt(1 + PC());
    //FIXME: Need to implement timing for each instruction for accurate simulation...
    // There are 2 ways to go here that I can see:
    // 1. Execute instruction, and then do nothing until X ticks have passed.
    // 2. Wait X ticks and then execute instruction.
    // 
    // 1. Seems a bit easier to implement, but I think there could be inaccuracies in simulation.
    // 2. Seems a bit harder, but is likely more accurate.
    switch (instruction) 
    {
        // CPU Memory and Register Transfers

        // Register to Register Transfer
        // A8        nz----  2   TAY         Transfer Accumulator to Y    Y=A
        case 0xA8:
            setY(A());
            setPC(PC() + 1);
            break;
        // AA        nz----  2   TAX         Transfer Accumulator to X    X=A
        case 0xAA:
            setX(A());
            setPC(PC() + 1);
            break;
        // BA        nz----  2   TSX         Transfer Stack pointer to X  X=S
        case 0xBA:
            setX(S());
            setPC(PC() + 1);
            break;
        // 98        nz----  2   TYA         Transfer Y to Accumulator    A=Y
        case 0x98:
            setA(Y());
            setPC(PC() + 1);
            break;
        // 8A        nz----  2   TXA         Transfer X to Accumulator    A=X
        case 0x8A:
            setA(X());
            setPC(PC() + 1);
            break;
        // 9A        ------  2   TXS         Transfer X to Stack pointer  S=X  
        case 0x9A:
            setS(X());
            setPC(PC() + 1);
            break;

        // Load Register from Memory
        // A9 nn     nz----  2   LDA #nn     Load A with Immediate     A=nn
        case 0xA9:
            setA(operand1);
            setPC(PC() + 2);
            break;
        // A5 nn     nz----  3   LDA nn      Load A with Zero Page     A=[nn]
        case 0xA5:
            setA(m_memory.byteAt(operand1));
            setPC(PC() + 2);
            break;
        // B5 nn     nz----  4   LDA nn,X    Load A with Zero Page,X   A=[nn+X]
        case 0xB5:
            setA(m_memory.byteAt(operand1 + X()));
            setPC(PC() + 2);
            break; 
        // AD nn nn  nz----  4   LDA nnnn    Load A with Absolute      A=[nnnn]
        case 0xAD:
            setA(m_memory.byteAt(wordOperand));
            setPC(PC() + 3);
            break;
        // BD nn nn  nz----  4*  LDA nnnn,X  Load A with Absolute,X    A=[nnnn+X]
        case 0xBD:
            setA(m_memory.byteAt(wordOperand + X()));
            setPC(PC() + 3);
            break;
        // B9 nn nn  nz----  4*  LDA nnnn,Y  Load A with Absolute,Y    A=[nnnn+Y]
        case 0xB9:
            setA(m_memory.byteAt(wordOperand + Y()));
            setPC(PC() + 3);
            break;
        // A1 nn     nz----  6   LDA (nn,X)  Load A with (Indirect,X)  A=[WORD[nn+X]]
        case 0xA1:
            setA(m_memory.byteAt(m_memory.wordAt(operand1 + X())));
            setPC(PC() + 2);
            break;
        // B1 nn     nz----  5*  LDA (nn),Y  Load A with (Indirect),Y  A=[WORD[nn]+Y]
        case 0xB1:
            setA(m_memory.byteAt(m_memory.wordAt(operand1) + Y()));
            setPC(PC() + 2);
            break;
        // A2 nn     nz----  2   LDX #nn     Load X with Immediate     X=nn
        case 0xA2:
            setX(operand1);
            setPC(PC() + 2);
            break;
        // A6 nn     nz----  3   LDX nn      Load X with Zero Page     X=[nn]
        case 0xA6:
            setX(m_memory.byteAt(operand1));
            setPC(PC() + 2);
            break;
        // B6 nn     nz----  4   LDX nn,Y    Load X with Zero Page,Y   X=[nn+Y]
        case 0xB6:
            setX(m_memory.byteAt(operand1 + Y()));
            setPC(PC() + 2);
            break;
        // AE nn nn  nz----  4   LDX nnnn    Load X with Absolute      X=[nnnn]
        case 0xAE:
            setX(m_memory.byteAt(wordOperand));
            setPC(PC() + 3);
            break;
        // BE nn nn  nz----  4*  LDX nnnn,Y  Load X with Absolute,Y    X=[nnnn+Y]
        case 0xBE:
            setX(m_memory.byteAt(wordOperand + Y()));
            setPC(PC() + 3);
            break;
        // A0 nn     nz----  2   LDY #nn     Load Y with Immediate     Y=nn
        case 0xA0:
            setY(operand1);
            setPC(PC() + 2);
            break;
        // A4 nn     nz----  3   LDY nn      Load Y with Zero Page     Y=[nn]
        case 0xA4:
            setY(m_memory.byteAt(operand1));
            setPC(PC() + 2);
            break;
        // B4 nn     nz----  4   LDY nn,X    Load Y with Zero Page,X   Y=[nn+X]
        case 0xB4:
            setY(m_memory.byteAt(operand1 + X()));
            setPC(PC() + 2);
            break;
        // AC nn nn  nz----  4   LDY nnnn    Load Y with Absolute      Y=[nnnn]
        case 0xAC:
            setY(m_memory.byteAt(wordOperand));
            setPC(PC() + 3);
            break;
        // BC nn nn  nz----  4*  LDY nnnn,X  Load Y with Absolute,X    Y=[nnnn+X]
        case 0xBC:
            setY(m_memory.byteAt(wordOperand + X()));
            setPC(PC() + 2);
            break;

        // Store Register in Memory
        // 85 nn     ------  3   STA nn      Store A in Zero Page     [nn]=A
        case 0x85:
            m_memory.byteAt(operand1) = A();
            setPC(PC() + 2);
            break;
        // 95 nn     ------  4   STA nn,X    Store A in Zero Page,X   [nn+X]=A
        case 0x95:
            m_memory.byteAt(operand1 + X()) = A();
            setPC(PC() + 2);
            break;
        // 8D nn nn  ------  4   STA nnnn    Store A in Absolute      [nnnn]=A
        case 0x8D:
            m_memory.byteAt(wordOperand) = A();
            setPC(PC() + 3);
            break;
        // 9D nn nn  ------  5   STA nnnn,X  Store A in Absolute,X    [nnnn+X]=A
        case 0x9D:
            m_memory.byteAt(wordOperand + X()) = A();
            setPC(PC() + 3);
            break;
        // 99 nn nn  ------  5   STA nnnn,Y  Store A in Absolute,Y    [nnnn+Y]=A
        case 0x99:
            m_memory.byteAt(wordOperand + Y()) = A();
            setPC(PC() + 3);
            break;
        // 81 nn     ------  6   STA (nn,X)  Store A in (Indirect,X)  [[nn+x]]=A
        case 0x81:
            m_memory.byteAt(m_memory.byteAt(operand1 + X())) = A();
            setPC(PC() + 2);
            break;
        // 91 nn     ------  6   STA (nn),Y  Store A in (Indirect),Y  [[nn]+y]=A
        case 0x91:
            m_memory.byteAt(m_memory.byteAt(operand1) + Y()) = A();
            setPC(PC() + 2);
            break;
        // 86 nn     ------  3   STX nn      Store X in Zero Page     [nn]=X
        case 0x86:
            m_memory.byteAt(operand1) = X();
            setPC(PC() + 2);
            break;
        // 96 nn     ------  4   STX nn,Y    Store X in Zero Page,Y   [nn+Y]=X
        case 0x96:
            m_memory.byteAt(operand1 + Y()) = X();
            setPC(PC() + 2);
            break;
        // 8E nn nn  ------  4   STX nnnn    Store X in Absolute      [nnnn]=X
        case 0x8E:
            m_memory.byteAt(wordOperand) = X();
            setPC(PC() + 3);
            break;
        // 84 nn     ------  3   STY nn      Store Y in Zero Page     [nn]=Y
        case 0x84:
            m_memory.byteAt(operand1) = Y();
            setPC(PC() + 2);
            break;
        // 94 nn     ------  4   STY nn,X    Store Y in Zero Page,X   [nn+X]=Y
        case 0x94:
            m_memory.byteAt(operand1 + X()) = Y();
            setPC(PC() + 2);
            break;
        // 8C nn nn  ------  4   STY nnnn    Store Y in Absolute      [nnnn]=Y
        case 0x8C:
            m_memory.byteAt(wordOperand) = Y();
            setPC(PC() + 3);
            break;

        // Push/Pull
        // 48        ------  3   PHA         Push accumulator on stack        [S]=A
        case 0x48:
            pushStackByte(A());
            setPC(PC() + 1);
            break;
        // 08        ------  3   PHP         Push processor status on stack   [S]=P
        case 0x08:
            pushStackByte(statusRegister().value());
            setPC(PC() + 1);
            break;
        // 68        nz----  4   PLA         Pull accumulator from stack      A=[S]
        case 0x68:
            setA(popStackByte());
            setPC(PC() + 1);
            break;
        // 28        nzcidv  4   PLP         Pull processor status from stack P=[S]
        case 0x28:
            setStatusRegister(StatusRegister(popStackByte()));
            setPC(PC() + 1);
            break;

        // Add memory to accumulator with carry
        // 69 nn     nzc--v  2   ADC #nn     Add Immediate           A=A+C+nn
        case 0x69:
            setA(additionWithCarry(A(), operand1));
            setPC(PC() + 2);
            break;
        // 65 nn     nzc--v  3   ADC nn      Add Zero Page           A=A+C+[nn]
        case 0x65:
            setA(additionWithCarry(A(), m_memory.byteAt(operand1)));
            setPC(PC() + 2);
            break;
        // 75 nn     nzc--v  4   ADC nn,X    Add Zero Page,X         A=A+C+[nn+X]
        case 0x75:
            setA(additionWithCarry(A(), m_memory.byteAt(operand1 + X())));
            setPC(PC() + 2);
            break;
        // 6D nn nn  nzc--v  4   ADC nnnn    Add Absolute            A=A+C+[nnnn]
        case 0x6D:
            setA(additionWithCarry(A(), m_memory.byteAt(wordOperand)));
            setPC(PC() + 3);
            break;
        // 7D nn nn  nzc--v  4*  ADC nnnn,X  Add Absolute,X          A=A+C+[nnnn+X]
        case 0x7D:
            setA(additionWithCarry(A(), m_memory.byteAt(wordOperand + X())));
            setPC(PC() + 3);
            break;
        // 79 nn nn  nzc--v  4*  ADC nnnn,Y  Add Absolute,Y          A=A+C+[nnnn+Y]
        case 0x79:
            setA(additionWithCarry(A(), m_memory.byteAt(wordOperand + Y())));
            setPC(PC() + 3);
            break;
        // 61 nn     nzc--v  6   ADC (nn,X)  Add (Indirect,X)        A=A+C+[[nn+X]]
        case 0x61:
            setA(additionWithCarry(A(), m_memory.byteAt(m_memory.wordAt(operand1 + X()))));
            setPC(PC() + 2);
            break;
        // 71 nn     nzc--v  5*  ADC (nn),Y  Add (Indirect),Y        A=A+C+[[nn]+Y]
        case 0x71:
            setA(additionWithCarry(A(), m_memory.byteAt(m_memory.wordAt(operand1) + Y())));
            setPC(PC() + 2);
            break;
        // * Add one cycle if indexing crosses a page boundary.

        // Subtract memory from accumulator with borrow
        // E9 nn     nzc--v  2   SBC #nn     Subtract Immediate      A=A+C-1-nn
        case 0xE9:
            setA(subtractionWithBorrow(A(), operand1));
            setPC(PC() + 2);
            break;
        // E5 nn     nzc--v  3   SBC nn      Subtract Zero Page      A=A+C-1-[nn]
        case 0xE5:
            setA(subtractionWithBorrow(A(), m_memory.byteAt(operand1)));
            setPC(PC() + 2);
            break;
        // F5 nn     nzc--v  4   SBC nn,X    Subtract Zero Page,X    A=A+C-1-[nn+X]
        case 0xF5:
            setA(subtractionWithBorrow(A(), m_memory.byteAt(operand1 + X())));
            setPC(PC() + 2);
            break;
        // ED nn nn  nzc--v  4   SBC nnnn    Subtract Absolute       A=A+C-1-[nnnn]
        case 0xED:
            setA(subtractionWithBorrow(A(), m_memory.byteAt(wordOperand)));
            setPC(PC() + 3);
            break;
        // FD nn nn  nzc--v  4*  SBC nnnn,X  Subtract Absolute,X     A=A+C-1-[nnnn+X]
        case 0xFD:
            setA(subtractionWithBorrow(A(), m_memory.byteAt(wordOperand + X())));
            setPC(PC() + 3);
            break;
        // F9 nn nn  nzc--v  4*  SBC nnnn,Y  Subtract Absolute,Y     A=A+C-1-[nnnn+Y]
        case 0xF9:
            setA(subtractionWithBorrow(A(), m_memory.byteAt(wordOperand + Y())));
            setPC(PC() + 3);
            break;
        // E1 nn     nzc--v  6   SBC (nn,X)  Subtract (Indirect,X)   A=A+C-1-[[nn+X]]
        case 0xE1:
            setA(subtractionWithBorrow(A(), m_memory.byteAt(m_memory.wordAt(operand1 + X()))));
            setPC(PC() + 2);
            break;
        // F1 nn     nzc--v  5*  SBC (nn),Y  Subtract (Indirect),Y   A=A+C-1-[[nn]+Y]
        case 0xF1:
            setA(subtractionWithBorrow(A(), m_memory.byteAt(m_memory.wordAt(operand1) + Y())));
            setPC(PC() + 2);
            break;
        // * Add one cycle if indexing crosses a page boundary.
        // FIXME: I'm not quite sure what to make of this note...
        // Note: Compared with normal 80x86 and Z80 CPUs, incoming and resulting Carry Flag are reversed.

        // Logical AND memory with accumulator
        // 29 nn     nz----  2   AND #nn     AND Immediate      A=A AND nn
        case 0x29:
            setA(A() & operand1);
            setPC(PC() + 2);
            break;
        // 25 nn     nz----  3   AND nn      AND Zero Page      A=A AND [nn]
        case 0x25:
            setA(A() & m_memory.byteAt(operand1));
            setPC(PC() + 2);
            break;
        // 35 nn     nz----  4   AND nn,X    AND Zero Page,X    A=A AND [nn+X]
        case 0x35:
            setA(A() & m_memory.byteAt(operand1 + X()));
            setPC(PC() + 2);
            break;
        // 2D nn nn  nz----  4   AND nnnn    AND Absolute       A=A AND [nnnn]
        case 0x2D:
            setA(A() & m_memory.byteAt(wordOperand));
            setPC(PC() + 3);
            break;
        // 3D nn nn  nz----  4*  AND nnnn,X  AND Absolute,X     A=A AND [nnnn+X]
        case 0x3D:
            setA(A() & m_memory.byteAt(wordOperand + X()));
            setPC(PC() + 3);
            break;
        // 39 nn nn  nz----  4*  AND nnnn,Y  AND Absolute,Y     A=A AND [nnnn+Y]
        case 0x39:
            setA(A() & m_memory.byteAt(wordOperand + Y()));
            setPC(PC() + 3);
            break;
        // 21 nn     nz----  6   AND (nn,X)  AND (Indirect,X)   A=A AND [[nn+X]]
        case 0x21:
            setA(A() & m_memory.byteAt(m_memory.wordAt(operand1 + X())));
            setPC(PC() + 2);
            break;
        // 31 nn     nz----  5*  AND (nn),Y  AND (Indirect),Y   A=A AND [[nn]+Y]
        case 0x31:
            setA(A() & m_memory.byteAt(m_memory.wordAt(operand1) + Y()));
            setPC(PC() + 2);
            break;
        // * Add one cycle if indexing crosses a page boundary.

        // Exclusive-OR memory with accumulator
        // 49 nn     nz----  2   EOR #nn     XOR Immediate      A=A XOR nn
        case 0x49:
            setA(A() ^ operand1);
            setPC(PC() + 2);
            break;
        // 45 nn     nz----  3   EOR nn      XOR Zero Page      A=A XOR [nn]
        case 0x45:
            setA(A() ^ m_memory.byteAt(operand1));
            setPC(PC() + 2);
            break;
        // 55 nn     nz----  4   EOR nn,X    XOR Zero Page,X    A=A XOR [nn+X]
        case 0x55:
            setA(A() ^ m_memory.byteAt(operand1 + X()));
            setPC(PC() + 2);
            break;
        // 4D nn nn  nz----  4   EOR nnnn    XOR Absolute       A=A XOR [nnnn]
        case 0x4D:
            setA(A() ^ m_memory.byteAt(wordOperand));
            setPC(PC() + 3);
            break;
        // 5D nn nn  nz----  4*  EOR nnnn,X  XOR Absolute,X     A=A XOR [nnnn+X]
        case 0x5D:
            setA(A() ^ m_memory.byteAt(wordOperand + X()));
            setPC(PC() + 3);
            break;
        // 59 nn nn  nz----  4*  EOR nnnn,Y  XOR Absolute,Y     A=A XOR [nnnn+Y]
        case 0x59:
            setA(A() ^ m_memory.byteAt(wordOperand + Y()));
            setPC(PC() + 3);
            break;
        // 41 nn     nz----  6   EOR (nn,X)  XOR (Indirect,X)   A=A XOR [[nn+X]]
        case 0x41:
            setA(A() ^ m_memory.byteAt(m_memory.wordAt(operand1 + X())));
            setPC(PC() + 2);
            break;
        // 51 nn     nz----  5*  EOR (nn),Y  XOR (Indirect),Y   A=A XOR [[nn]+Y]
        case 0x51:
            setA(A() ^ m_memory.byteAt(m_memory.wordAt(operand1) + Y()));
            setPC(PC() + 2);
            break;
        // * Add one cycle if indexing crosses a page boundary.

        // Logical OR memory with accumulator
        // 09 nn     nz----  2   ORA #nn     OR Immediate       A=A OR nn
        case 0x09:
            setA(A() | operand1);
            setPC(PC() + 2);
            break;
        // 05 nn     nz----  3   ORA nn      OR Zero Page       A=A OR [nn]
        case 0x05:
            setA(A() | m_memory.byteAt(operand1));
            setPC(PC() + 2);
            break;
        // 15 nn     nz----  4   ORA nn,X    OR Zero Page,X     A=A OR [nn+X]
        case 0x15:
            setA(A() | m_memory.byteAt(operand1 + X()));
            setPC(PC() + 2);
            break;
        // 0D nn nn  nz----  4   ORA nnnn    OR Absolute        A=A OR [nnnn]
        case 0x0D:
            setA(A() | m_memory.byteAt(wordOperand));
            setPC(PC() + 3);
            break;
        // 1D nn nn  nz----  4*  ORA nnnn,X  OR Absolute,X      A=A OR [nnnn+X]
        case 0x1D:
            setA(A() | m_memory.byteAt(wordOperand + X()));
            setPC(PC() + 3);
            break;
        // 19 nn nn  nz----  4*  ORA nnnn,Y  OR Absolute,Y      A=A OR [nnnn+Y]
        case 0x19:
            setA(A() | m_memory.byteAt(wordOperand + Y()));
            setPC(PC() + 3);
            break;
        // 01 nn     nz----  6   ORA (nn,X)  OR (Indirect,X)    A=A OR [[nn+X]]
        case 0x01:
            setA(A() | m_memory.byteAt(m_memory.wordAt(operand1 + X())));
            setPC(PC() + 2);
            break;
        // 11 nn     nz----  5*  ORA (nn),Y  OR (Indirect),Y    A=A OR [[nn]+Y]
        case 0x11:
            setA(A() | m_memory.byteAt(m_memory.wordAt(operand1) + Y()));
            setPC(PC() + 2);
            break;
        // * Add one cycle if indexing crosses a page boundary.

        // Compare
        // C9 nn     nzc---  2   CMP #nn     Compare A with Immediate     A-nn
        case 0xC9:
            compare(A(), operand1);
            setPC(PC() + 2);
            break;
        // C5 nn     nzc---  3   CMP nn      Compare A with Zero Page     A-[nn]
        case 0xC5:
            compare(A(), m_memory.byteAt(operand1));
            setPC(PC() + 2);
            break;
        // D5 nn     nzc---  4   CMP nn,X    Compare A with Zero Page,X   A-[nn+X]
        case 0xD5:
            compare(A(), m_memory.byteAt(operand1 + X()));
            setPC(PC() + 2);
            break;
        // CD nn nn  nzc---  4   CMP nnnn    Compare A with Absolute      A-[nnnn]
        case 0xCD:
            compare(A(), m_memory.byteAt(wordOperand));
            setPC(PC() + 3);
            break;
        // DD nn nn  nzc---  4*  CMP nnnn,X  Compare A with Absolute,X    A-[nnnn+X]
        case 0xDD:
            compare(A(), m_memory.byteAt(wordOperand + X()));
            setPC(PC() + 3);
            break;
        // D9 nn nn  nzc---  4*  CMP nnnn,Y  Compare A with Absolute,Y    A-[nnnn+Y]
        case 0xD9:
            compare(A(), m_memory.byteAt(wordOperand + Y()));
            setPC(PC() + 3);
            break;
        // C1 nn     nzc---  6   CMP (nn,X)  Compare A with (Indirect,X)  A-[[nn+X]]
        case 0xC1:
            compare(A(), m_memory.byteAt(m_memory.wordAt(operand1 + X())));
            setPC(PC() + 2);
            break;
        // D1 nn     nzc---  5*  CMP (nn),Y  Compare A with (Indirect),Y  A-[[nn]+Y]
        case 0xD1:
            compare(A(), m_memory.byteAt(m_memory.wordAt(operand1) + Y()));
            setPC(PC() + 2);
            break;
        // E0 nn     nzc---  2   CPX #nn     Compare X with Immediate     X-nn
        case 0xE0:
            compare(X(), operand1);
            setPC(PC() + 2);
            break;
        // E4 nn     nzc---  3   CPX nn      Compare X with Zero Page     X-[nn]
        case 0xE4:
            compare(X(), m_memory.byteAt(operand1));
            setPC(PC() + 2);
            break;
        // EC nn nn  nzc---  4   CPX nnnn    Compare X with Absolute      X-[nnnn]
        case 0xEC:
            compare(X(), m_memory.byteAt(wordOperand));
            setPC(PC() + 3);
            break;
        // C0 nn     nzc---  2   CPY #nn     Compare Y with Immediate     Y-nn
        case 0xC0:
            compare(Y(), operand1);
            setPC(PC() + 2);
            break;
        // C4 nn     nzc---  3   CPY nn      Compare Y with Zero Page     Y-[nn]
        case 0xC4:
            compare(Y(), m_memory.byteAt(operand1));
            setPC(PC() + 2);
            break;
        // CC nn nn  nzc---  4   CPY nnnn    Compare Y with Absolute      Y-[nnnn]
        case 0xCC:
            compare(Y(), m_memory.byteAt(wordOperand));
            setPC(PC() + 3);
            break;
        // * Add one cycle if indexing crosses a page boundary.
        //FIXME: Not sure what to make of this note... be careful!
        // Note: Compared with normal 80x86 and Z80 CPUs, resulting Carry Flag is reversed.

        // Bit Test
        // 24 nn     xz---x  3   BIT nn      Bit Test   A AND [nn], N=[nn].7, V=[nn].6
        case 0x24:
            bitTest(A(), m_memory.byteAt(operand1));
            setPC(PC() + 2);
            break;
        // 2C nn nn  xz---x  4   BIT nnnn    Bit Test   A AND [..], N=[..].7, V=[..].6
        case 0x2C:
            bitTest(A(), m_memory.byteAt(wordOperand));
            setPC(PC() + 3);
            break;

        // Increment by one
        // E6 nn     nz----  5   INC nn      Increment Zero Page    [nn]=[nn]+1
        case 0xE6:
            setByte(m_memory.byteAt(operand1), m_memory.byteAt(operand1) + 1);
            setPC(PC() + 2);
            break;
        // F6 nn     nz----  6   INC nn,X    Increment Zero Page,X  [nn+X]=[nn+X]+1
        case 0xF6:
            setByte(m_memory.byteAt(operand1 + X()), m_memory.byteAt(operand1 + X()) + 1);
            setPC(PC() + 2);
            break;
        // EE nn nn  nz----  6   INC nnnn    Increment Absolute     [nnnn]=[nnnn]+1
        case 0xEE:
            setByte(m_memory.byteAt(wordOperand), m_memory.byteAt(wordOperand) + 1);
            setPC(PC() + 3);
            break;
        // FE nn nn  nz----  7   INC nnnn,X  Increment Absolute,X   [nnnn+X]=[nnnn+X]+1
        case 0xFE:
            setByte(m_memory.byteAt(wordOperand + X()), m_memory.byteAt(wordOperand + X()) + 1);
            setPC(PC() + 3);
            break;
        // E8        nz----  2   INX         Increment X            X=X+1
        case 0xE8:
            setX(X() + 1);
            setPC(PC() + 1);
            break;
        // C8        nz----  2   INY         Increment Y            Y=Y+1
        case 0xC8:
            setY(Y() + 1);
            setPC(PC() + 1);
            break;

        // Decrement by one
        // C6 nn     nz----  5   DEC nn      Decrement Zero Page    [nn]=[nn]-1
        case 0xC6:
            setByte(m_memory.byteAt(operand1), m_memory.byteAt(operand1) - 1);
            setPC(PC() + 2);
            break;
        // D6 nn     nz----  6   DEC nn,X    Decrement Zero Page,X  [nn+X]=[nn+X]-1
        case 0xD6:
            setByte(m_memory.byteAt(operand1 + X()), m_memory.byteAt(wordOperand + X()) - 1);
            setPC(PC() + 2);
            break;
        // CE nn nn  nz----  6   DEC nnnn    Decrement Absolute     [nnnn]=[nnnn]-1
        case 0xCE:
            setByte(m_memory.byteAt(wordOperand), m_memory.byteAt(wordOperand) - 1);
            setPC(PC() + 3);
            break;
        // DE nn nn  nz----  7   DEC nnnn,X  Decrement Absolute,X   [nnnn+X]=[nnnn+X]-1
        case 0xDE:
            setByte(m_memory.byteAt(wordOperand + X()), m_memory.byteAt(wordOperand + X()) - 1);
            setPC(PC() + 3);
            break;
        // CA       nz----  2   DEX         Decrement X            X=X-1
        case 0xCA:
            setX(X() - 1);
            setPC(PC() + 1);
            break;
        // 88        nz----  2   DEY         Decrement Y            Y=Y-1
        case 0x88:
            setY(Y() - 1);
            setPC(PC() + 1);
            break;

        //Shift Left
        // 0A        nzc---  2   ASL A       Shift Left Accumulator   SHL A
        case 0x0A:
            setA(shiftLeft(A()));
            setPC(PC() + 1);
            break;
        // 06 nn     nzc---  5   ASL nn      Shift Left Zero Page     SHL [nn]
        case 0x06:
            setByte(m_memory.byteAt(operand1), shiftLeft(m_memory.byteAt(operand1)));
            setPC(PC() + 2);
            break;
        // 16 nn     nzc---  6   ASL nn,X    Shift Left Zero Page,X   SHL [nn+X]
        case 0x16:
            setByte(m_memory.byteAt(operand1 + X()), shiftLeft(m_memory.byteAt(operand1 + X())));
            setPC(PC() + 2);
            break;
        // 0E nn nn  nzc---  6   ASL nnnn    Shift Left Absolute      SHL [nnnn]
        case 0x0E:
            setByte(m_memory.byteAt(wordOperand), shiftLeft(m_memory.byteAt(wordOperand)));
            setPC(PC() + 3);
            break;
        // 1E nn nn  nzc---  7   ASL nnnn,X  Shift Left Absolute,X    SHL [nnnn+X]
        case 0x1E:
            setByte(m_memory.byteAt(wordOperand + X()), shiftLeft(m_memory.byteAt(wordOperand + X())));
            setPC(PC() + 3);
            break;

        // Shift Right
        // 4A        0zc---  2   LSR A       Shift Right Accumulator  SHR A
        case 0x4A:
            setA(shiftRight(A()));
            setPC(PC() + 1);
            break;
        // 46 nn     0zc---  5   LSR nn      Shift Right Zero Page    SHR [nn]
        case 0x46:
            setByte(m_memory.byteAt(operand1), shiftRight(operand1));
            setPC(PC() + 2);
            break;
        // 56 nn     0zc---  6   LSR nn,X    Shift Right Zero Page,X  SHR [nn+X]
        case 0x56:
            setByte(m_memory.byteAt(operand1 + X()), shiftRight(m_memory.byteAt(operand1 + X())));
            setPC(PC() + 2);
            break;
        // 4E nn nn  0zc---  6   LSR nnnn    Shift Right Absolute     SHR [nnnn]
        case 0x4E:
            setByte(m_memory.byteAt(wordOperand), shiftRight(m_memory.byteAt(wordOperand)));
            setPC(PC() + 3);
            break;
        // 5E nn nn  0zc---  7   LSR nnnn,X  Shift Right Absolute,X   SHR [nnnn+X]
        case 0x5E:
            setByte(m_memory.byteAt(wordOperand + X()), shiftRight(m_memory.byteAt(wordOperand + X())));
            setPC(PC() + 3);
            break;

        // Rotate Left through Carry
        // 2A        nzc---  2   ROL A       Rotate Left Accumulator  RCL A
        case 0x2A:
            setA(rotateLeftThroughCarry(A()));
            setPC(PC() + 1);
            break;
        // 26 nn     nzc---  5   ROL nn      Rotate Left Zero Page    RCL [nn]
        case 0x26:
            setByte(m_memory.byteAt(operand1), rotateLeftThroughCarry(m_memory.byteAt(operand1)));
            setPC(PC() + 2);
            break;
        // 36 nn     nzc---  6   ROL nn,X    Rotate Left Zero Page,X  RCL [nn+X]
        case 0x36:
            setByte(m_memory.byteAt(operand1 + X()), rotateLeftThroughCarry(m_memory.byteAt(operand1 + X())));
            setPC(PC() + 2);
            break;
        // 2E nn nn  nzc---  6   ROL nnnn    Rotate Left Absolute     RCL [nnnn]
        case 0x2E:
            setByte(m_memory.byteAt(wordOperand), rotateLeftThroughCarry(m_memory.byteAt(wordOperand)));
            setPC(PC() + 3);
            break;
        // 3E nn nn  nzc---  7   ROL nnnn,X  Rotate Left Absolute,X   RCL [nnnn+X]
        case 0x3E:
            setByte(m_memory.byteAt(wordOperand + X()), rotateLeftThroughCarry(m_memory.byteAt(wordOperand + X())));
            setPC(PC() + 3);
            break;

        // Rotate Right through Carry
        // 6A        nzc---  2   ROR A       Rotate Right Accumulator RCR A
        case 0x6A:
            setA(rotateRightThroughCarry(A()));
            setPC(PC() + 1);
            break;
        // 66 nn     nzc---  5   ROR nn      Rotate Right Zero Page   RCR [nn]
        case 0x66:
            setByte(m_memory.byteAt(operand1), rotateRightThroughCarry(m_memory.byteAt(operand1)));
            setPC(PC() + 2);
            break;
        // 76 nn     nzc---  6   ROR nn,X    Rotate Right Zero Page,X RCR [nn+X]
        case 0x76:
            setByte(m_memory.byteAt(operand1 + X()), rotateRightThroughCarry(m_memory.byteAt(operand1 + X())));
            setPC(PC() + 2);
            break;
        // 6E nn nn  nzc---  6   ROR nnnn    Rotate Right Absolute    RCR [nnnn]
        case 0x6E:
            setByte(m_memory.byteAt(wordOperand), rotateRightThroughCarry(m_memory.byteAt(wordOperand)));
            setPC(PC() + 3);
            break;
        // 7E nn nn  nzc---  7   ROR nnnn,X  Rotate Right Absolute,X  RCR [nnnn+X]
        case 0x7E:
            setByte(m_memory.byteAt(wordOperand + X()), rotateRightThroughCarry(m_memory.byteAt(wordOperand + X())));
            setPC(PC() + 3);
            break;

        // Normal Jumps
        // 4C nn nn  ------  3   JMP nnnn    Jump Absolute              PC=nnnn
        case 0x4C:
            setPC(wordOperand);
            break;
        // Glitch: For JMP [nnnn] the operand word cannot cross page boundaries, ie. JMP [03FFh] would 
        // fetch the MSB from [0300h] instead of [0400h]. Very simple workaround would be to place a ALIGN 2 before the data word.
        // FIXME: Implement Glitch above?
        // 6C nn nn  ------  5   JMP (nnnn)  Jump Indirect              PC=WORD[nnnn]
        case 0x6C:
            setPC(m_memory.wordAt(wordOperand));
            break;
        // 20 nn nn  ------  6   JSR nnnn    Jump and Save Return Addr. [S]=PC+2,PC=nnnn
        case 0x20:
            pushStackWord(PC() + 2);
            setPC(wordOperand);
            break;
        // Note: RTI cannot modify the B-Flag or the unused flag.
        // 40        nzcidv  6   RTI         Return from BRK/IRQ/NMI    P=[S], PC=[S]
        case 0x40:
        {
            bool breakFlag = m_status.breakFlag();
            setStatusRegister(StatusRegister(popStackByte()));
            m_status.setBreakFlag(breakFlag);
            setPC(popStackWord());
        }
            break;
        // 60        ------  6   RTS         Return from Subroutine     PC=[S]+1
        case 0x60:
            setPC(popStackWord() + 1);
            break;

        // Conditional Branches
        // ** The execution time is 2 cycles if the condition is false (no branch executed). Otherwise, 
        // 3 cycles if the destination is in the same memory page, or 4 cycles if it crosses a page boundary (see below for exact info).
        // Note: After subtractions (SBC or CMP) carry=set indicates above-or-equal, unlike as for 80x86 and Z80 CPUs. 
        // Obviously, this still applies even when using 80XX-style syntax.

        // Conditional Branch Page Crossing
        // The branch opcode with parameter takes up two bytes, causing the PC to get incremented twice (PC=PC+2), 
        // without any extra boundary cycle. The signed parameter is then added to the PC (PC+disp), the extra clock 
        // cycle occurs if the addition crosses a page boundary (next or previous 100h-page).

        // 10 dd     ------  2** BPL disp    Branch on result plus     if N=0 PC=PC+/-nn
        case 0x10:
            conditionalBranch(!m_status.negative(), operand1); 
            break;
        // 30 dd     ------  2** BMI disp    Branch on result minus    if N=1 PC=PC+/-nn
        case 0x30:
            conditionalBranch(m_status.negative(), operand1); 
            break;
        // 50 dd     ------  2** BVC disp    Branch on overflow clear  if V=0 PC=PC+/-nn
        case 0x50:
            conditionalBranch(!m_status.overflow(), operand1); 
            break;
        // 70 dd     ------  2** BVS disp    Branch on overflow set    if V=1 PC=PC+/-nn
        case 0x70:
            conditionalBranch(m_status.overflow(), operand1); 
            break;
        // 90 dd     ------  2** BCC disp    Branch on carry clear     if C=0 PC=PC+/-nn
        case 0x90:
            conditionalBranch(!m_status.carry(), operand1); 
            break;
        // B0 dd     ------  2** BCS disp    Branch on carry set       if C=1 PC=PC+/-nn
        case 0xB0:
            conditionalBranch(m_status.carry(), operand1);
            break;
        // D0 dd     ------  2** BNE disp    Branch on result not zero if Z=0 PC=PC+/-nn
        case 0xD0:
            conditionalBranch(!m_status.zero(), operand1);
            break;
        // F0 dd     ------  2** BEQ disp    Branch on result zero     if Z=1 PC=PC+/-nn
        case 0xF0:
            conditionalBranch(m_status.zero(), operand1);
            break;

        // Interrupts, Exceptions, Breakpoints
        // 00        ---1--  7   BRK   Force Break B=1 [S]=PC+1,[S]=P,I=1,PC=[FFFE]
        case 0x00:
            m_status.setBreakFlag(true);
            pushStackWord(PC() + 1);
            pushStackByte(m_status.value());
            m_status.setIRQDisable(true);
            setPC(m_memory.wordAt(0xFFFE));
            break;

        // FIXME: Not quite sure how to service the following requests... yet.
        // --        ---1--  ??  /IRQ  Interrupt   B=0 [S]=PC,[S]=P,I=1,PC=[FFFE]
        // --        ---1--  ??  /NMI  NMI         B=0 [S]=PC,[S]=P,I=1,PC=[FFFA]
        // --        ---1--  T+6 /RESET Reset      PC=[FFFC],I=1
        // Notes: IRQs can be disabled by setting the I-flag, a BRK command, a NMI, and a /RESET signal cannot be masked by setting I.
        // BRK/IRQ/NMI first change the B-flag, then write P to stack, and then set the I-flag, the D-flag is NOT changed and should be cleared by software.
        // The same vector is shared for BRK and IRQ, software can separate between BRK and IRQ by examining the pushed B-flag only.
        // The RTI opcode can be used to return from BRK/IRQ/NMI, note that using the return address 
        // from BRK skips one dummy/parameter byte following after the BRK opcode.
        // Software or hardware must take care to acknowledge or reset /IRQ or /NMI signals after processing it.
        // IRQs are executed whenever "/IRQ=LOW AND I=0".
        // NMIs are executed whenever "/NMI changes from HIGH to LOW".
        // If /IRQ is kept LOW then same (old) interrupt is executed again as soon as setting I=0. If /NMI is kept LOW then no further NMIs can be executed.

        // CPU Control
        // 18        --0---  2   CLC         Clear carry flag            C=0
        case 0x18:
            m_status.setCarry(false);
            setPC(PC() + 1);
            break;
        // 58        ---0--  2   CLI         Clear interrupt disable bit I=0
        case 0x58:
            m_status.setIRQDisable(false);
            setPC(PC() + 1);
            break;
        // D8        ----0-  2   CLD         Clear decimal mode          D=0
        case 0xD8:
            m_status.setDecimalMode(false);
            setPC(PC() + 1);
            break;
        // B8        -----0  2   CLV         Clear overflow flag         V=0
        case 0xB8:
            m_status.setOverflow(false);
            setPC(PC() + 1);
            break;
        // 38        --1---  2   SEC         Set carry flag              C=1
        case 0x38:
            m_status.setCarry(true);
            setPC(PC() + 1);
            break;
        // 78        ---1--  2   SEI         Set interrupt disable bit   I=1
        case 0x78:
            m_status.setIRQDisable(true);
            setPC(PC() + 1);
            break;
        // F8        ----1-  2   SED         Set decimal mode            D=1
        case 0xF8:
            m_status.setDecimalMode(true);
            setPC(PC() + 1);
            break;

        // No Operation
        // EA        ------  2   NOP         No operation                No operation
        case 0xEA:
            setPC(PC() + 2);
            break;
    }
}

// Cpu65XX accessors
const unsigned char &
Cpu65XX::
A() const
{
    return m_A;
}

const unsigned char &
Cpu65XX::
X() const
{
    return m_X;
}

const unsigned char &
Cpu65XX::
Y() const
{
    return m_Y;
}

const unsigned short &
Cpu65XX::
PC() const
{
    return m_PC;
}

const unsigned char &
Cpu65XX::
S() const
{
    return m_S;
}

unsigned short
Cpu65XX::
stackPointer() const
{
    return 0x0100 + static_cast<unsigned short>(S());
}

const Cpu65XX::StatusRegister&
Cpu65XX::
statusRegister() const
{
    return m_status;
}

std::string
Cpu65XX::
state() const 
{
    std::stringstream output;
    output << std::hex << "A:  " << static_cast<unsigned short>(m_A) << std::endl
           << "X:  " << static_cast<unsigned short>(m_X)  << std::endl
           << "Y:  " << static_cast<unsigned short>(m_Y)  << std::endl
           << "PC: " << static_cast<unsigned short>(m_PC) << std::endl
           << "S:  " << static_cast<unsigned short>(m_S)  << std::endl
           << "P:  " << static_cast<unsigned short>(m_status.value()) << std::endl
           << "CZIDBON" << std::endl
           << m_status.carry() << m_status.zero() << m_status.IRQDisable() << m_status.decimalMode()
           << m_status.breakFlag() << m_status.overflow() << m_status.negative() << std::endl;

    return output.str();
}

// Cpu65XX mutators
void
Cpu65XX::
setA(const unsigned char& value) 
{
    handleRegisterAssignmentFlags(value);
    m_A = value;
}

void
Cpu65XX::
setX(const unsigned char& value) 
{
    handleRegisterAssignmentFlags(value);
    m_X = value;
}

void
Cpu65XX::
setY(const unsigned char& value) 
{
    handleRegisterAssignmentFlags(value);
    m_Y = value;
}

void
Cpu65XX::
setPC(const unsigned short& value) 
{
    m_PC = value;
}

void
Cpu65XX::
setS(const unsigned char& value) 
{
    m_S = value;
}

void
Cpu65XX::
setStatusRegister(const StatusRegister& status) 
{
    m_status = status;
}

void
Cpu65XX::
handleRegisterAssignmentFlags(const unsigned char& value) 
{
    // If the value is zero, then the zero flag is set.
    // If the value is not zero, then the zero flag is zero, go figure!
    m_status.setZero(0x00 == value);
    // The highest bit determines whether the value is negative or not.
    m_status.setNegative((value & 0x80) > 0);
}

//FIXME: Verify
unsigned char 
Cpu65XX::
additionWithCarry(const unsigned char& op1, const unsigned char& op2)
{
    //FIXME: Surely we can make this more efficient...
    unsigned short result = static_cast<unsigned short>(op1) + static_cast<unsigned char>(m_status.carry()) + op2;
    m_status.setCarry(result > 0xFF);
    short signedResult = static_cast<short>(op1) + static_cast<short>(m_status.carry()) + static_cast<short>(op1);
    m_status.setOverflow(signedResult > 127 || signedResult < -128);
    return static_cast<unsigned char>(result);
}

//FIXME: Verify
unsigned char 
Cpu65XX::
subtractionWithBorrow(const unsigned char& op1, const unsigned char& op2)
{
    unsigned short result = op1 + m_status.carry() - 1 - op2;
    m_status.setCarry(result > op1);
    short signedResult = static_cast<short>(result);
    m_status.setOverflow(signedResult > 127 || signedResult < -128);
    return static_cast<unsigned char>(result);
}

//FIXME: Verify
void          
Cpu65XX::
compare(const unsigned char& accumulator, const unsigned char& memory)
{
    unsigned char result = accumulator - memory;
    m_status.setZero(result == 0);
    m_status.setCarry(result > accumulator);
    m_status.setNegative(result & 0x80);
}

//FIXME: Verify
unsigned char 
Cpu65XX::
shiftLeft(const unsigned char& op)
{
    unsigned char result = op << 1;
    m_status.setCarry(op & 0x80);
    return result;
}

//FIXME: Verify
unsigned char 
Cpu65XX::
shiftRight(const unsigned char& op)
{
    unsigned char result = op >> 1;
    m_status.setCarry(op & 0x01);
    return result;
}

//FIXME: Verify
unsigned char 
Cpu65XX::
rotateLeftThroughCarry(const unsigned char& op)
{
    unsigned char result = (op << 1) + m_status.carry();
    m_status.setCarry(op & 0x80);
    return result;
}

//FIXME: Verify
unsigned char 
Cpu65XX::
rotateRightThroughCarry(const unsigned char& op)
{
    unsigned char result = (op >> 1) + (m_status.carry() * 0x80);
    m_status.setCarry(op & 0x01);
    return result;
}

void
Cpu65XX::
setByte(unsigned char& destination, const unsigned char& value)
{
    destination = value;
    m_status.setNegative(destination & 0x80);
    m_status.setZero(destination == 0);
}

void
Cpu65XX::
bitTest(const unsigned char& op1, const unsigned char& op2) 
{
    unsigned char result = op1 & op2;
    m_status.setNegative(result & 0x80);
    m_status.setOverflow(result & 0x40);
    m_status.setZero(result == 0);
}

void
Cpu65XX::
conditionalBranch(bool condition, const unsigned char& destination)
{
    if (condition)
    {
        setPC(PC() + 2 + static_cast<signed char>(destination));
    }
}

void 
Cpu65XX::
pushStackByte(const unsigned char& value) 
{
    m_memory.byteAt(stackPointer()) = value;
    setS(S() - 1);
}

void 
Cpu65XX::
pushStackWord(const unsigned short& value)
{
    m_memory.wordAt(stackPointer()) = value;
    setS(S() - 2);
}

unsigned char&
Cpu65XX::
popStackByte()
{
    setS(S() + 1);
    return m_memory.byteAt(stackPointer() - 1);
}

unsigned short& 
Cpu65XX::
popStackWord()
{
    setS(S() + 2);
    return m_memory.wordAt(stackPointer() - 2);
}

// Begin Memory implementation
Cpu65XX::Memory::
Memory()
{
    std::fill(m_memory, m_memory + (MAIN_MEM_SIZE), 0x00);
}

Cpu65XX::Memory::
Memory(unsigned char * toLoad, unsigned int size)
{
    std::fill(m_memory, m_memory + (MAIN_MEM_SIZE), 0x00);
    std::copy(toLoad, toLoad + size, m_memory);
}


//FIXME: Consider what to do when the address is out-of-bounds.
// Throw an expection? 

// Cpu65XX::Memory mutators
unsigned char&
Cpu65XX::Memory::
byteAt(const unsigned short& address) 
{
    //FIXME: ensure that address is treated as little-endian
    //Meaning that an address of ABCD is represented by 0xCD AB 
    //in memory!
    return m_memory[address];
}

unsigned short&
Cpu65XX::Memory::
wordAt(const unsigned short& address) 
{
    //FIXME: Just how hacky and bad is this? Lets find out...
    return *(static_cast<unsigned short*>(static_cast<void*>(m_memory + address)));
}

// End Memory implementation


// Begin StatusRegister Implementation
Cpu65XX::StatusRegister::
StatusRegister() 
{
    // This flag is not used but always 1.
    m_status[5] = 1;
}


Cpu65XX::StatusRegister::
StatusRegister(unsigned char& value) 
{
    m_status[0] = value & 0x01;
    m_status[1] = value & 0x02;
    m_status[2] = value & 0x04;
    m_status[3] = value & 0x08;
    m_status[4] = value & 0x10;
    m_status[5] = 1;
    m_status[6] = value & 0x40;
    m_status[7] = value & 0x80;
}

// StatusRegister accessors
bool
Cpu65XX::StatusRegister::
carry() const
{
    return m_status[0];
}

bool
Cpu65XX::StatusRegister::
zero() const
{
    return m_status[1];
}

bool
Cpu65XX::StatusRegister::
IRQDisable() const
{
    return m_status[2];
}

bool
Cpu65XX::StatusRegister::
decimalMode() const
{
    return m_status[3];
}

bool
Cpu65XX::StatusRegister::
breakFlag() const
{
    return m_status[4];
}

bool
Cpu65XX::StatusRegister::
overflow() const
{
    return m_status[6];
}

bool
Cpu65XX::StatusRegister::
negative() const
{
    return m_status[7];
}

unsigned char
Cpu65XX::StatusRegister::
value() const
{
    return static_cast<unsigned char>(m_status.to_ulong());
}

// StatusRegister mutators

void
Cpu65XX::StatusRegister::
setCarry(bool value) 
{
    m_status[0] = value;    
}

void
Cpu65XX::StatusRegister::
setZero(bool value) 
{
    m_status[1] = value;    
}

void
Cpu65XX::StatusRegister::
setIRQDisable(bool value) 
{
    m_status[2] = value;    
}

void
Cpu65XX::StatusRegister::
setDecimalMode(bool value) 
{
    m_status[3] = value;    
}

void
Cpu65XX::StatusRegister::
setBreakFlag(bool value) 
{
    m_status[4] = value;    
}

void
Cpu65XX::StatusRegister::
setOverflow(bool value) 
{
    m_status[6] = value;    
}

void
Cpu65XX::StatusRegister::
setNegative(bool value) 
{
    m_status[7] = value;    
}
// End StatusRegister implementation.







