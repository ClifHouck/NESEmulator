#include "Cpu65XX.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cassert>

// For some reason the stack appears to startup at 0xFD
const u8_byte initialStackValue = 0xFD;

Cpu65XX::
Cpu65XX(Memory& memory) :
    PoweredDevice(this),
    ClockedDevice(clockDivisor),
    m_memory (memory),
    m_A (0),
    m_X (0),
    m_Y (0),
    m_PC (0),
    m_S (initialStackValue),
    m_NMI (false),
    m_IRQ (false),
    m_downCycles (0),
    m_cycles     (0),
    m_queuedInstruction (0),
    m_illegalInstructions ()
{
    m_instructions = new Instruction[256];
    buildMemoryOperationFuncs();
    buildDisassemblyFunctions();
    buildInstructionSet();
}

Cpu65XX::
~Cpu65XX() 
{
    delete[] m_instructions;
    m_instructions = nullptr;
}

u16_word
zeroPage(u16_word address) 
{
    return address & 0x00FF;
}

void
Cpu65XX::
buildDisassemblyFunctions() 
{
    m_disassemblyFunctions["None"]      = [] { return std::string(); };
    m_disassemblyFunctions["A"]         = [] { return std::string("A"); };
    m_disassemblyFunctions["Branch"] = [this] {
        std::stringstream output;
        output.fill('0');
        u16_word destination = PC() + 2 + static_cast<signed char>(byteOperand());
        output << std::hex << "$" << std::setw(4) << destination;
        return output.str();
    };
    m_disassemblyFunctions["Immediate"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "#$" << std::setw(2) << (int)byteOperand();
        return output.str();
    };
    m_disassemblyFunctions["Indirect"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "($" << std::setw(4) << (int)wordOperand() << ") = "
               << std::setw(4) << (int)m_memory.rawReadWord(wordOperand());
        return output.str();
    };
    m_disassemblyFunctions["ZeroPage"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "$" << std::setw(2) << (int)byteOperand() << " = " << std::setw(2) << (int)m_memory.rawReadByte(byteOperand());
        return output.str();
    };
    m_disassemblyFunctions["ZeroPage,X"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "$" 
               << std::setw(2) << (int)byteOperand() << ",X @ " 
               << std::setw(2) << (int)(byteOperand() + X()) % 0x100 << " = " 
               << std::setw(2) << (int)m_memory.rawReadByte(zeroPage(byteOperand() + X()));
        return output.str();
    };
    m_disassemblyFunctions["ZeroPage,Y"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "$" 
               << std::setw(2) << (int)byteOperand() << ",Y @ " 
               << std::setw(2) << (int)(byteOperand() + Y()) % 0x100 << " = " 
               << std::setw(2) << (int)m_memory.rawReadByte(zeroPage(byteOperand() + Y()));
        return output.str();
    };
    m_disassemblyFunctions["Absolute"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "$" 
               << std::setw(4) << (int)wordOperand();
        return output.str();
    };
    m_disassemblyFunctions["AbsoluteWithValue"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "$" 
               << std::setw(4) << (int)wordOperand()
               << " = "  << std::setw(2) << (int)m_memory.rawReadByte(wordOperand());
        return output.str();
    };
    m_disassemblyFunctions["AbsoluteMem"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "$" 
               << std::setw(4) << (int)wordOperand()
               << " = "  << std::setw(2) << (int)m_memory.rawReadByte(wordOperand());
        return output.str();
    };
    m_disassemblyFunctions["Absolute,X"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "$" 
               << std::setw(4) << (int)wordOperand() << ",X @ " 
               << std::setw(4) << (int)(wordOperand() + X()) << " = " 
               << std::setw(2) << (int)m_memory.rawReadByte(wordOperand() + X());
        return output.str();
    };
    m_disassemblyFunctions["Absolute,Y"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "$" 
               << std::setw(4) << (int)wordOperand() << ",Y @ " 
               << std::setw(4) << (int)(wordOperand() + Y()) % 0x10000 << " = " 
               << std::setw(2) << (int)m_memory.rawReadByte(wordOperand() + Y());
        return output.str();
    };
    m_disassemblyFunctions["(Indirect,X)"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "($" 
               << std::setw(2) << (int)byteOperand() << ",X) @ " 
               << std::setw(2) << (int)((u8_byte)(byteOperand() + X())) << " = " 
               << std::setw(4) << (int)wordAt(zeroPage(byteOperand() + X())) << " = "
               << std::setw(2) << (int)m_memory.rawReadByte(wordAt(zeroPage(byteOperand() + X())));
        return output.str();
    };
    m_disassemblyFunctions["(Indirect),Y"] = [this] {
        std::stringstream output;
        output.fill('0');
        output << std::hex << "($" 
               << std::setw(2) << (int)byteOperand() << "),Y = " 
               << std::setw(4) << (int)m_memory.rawReadWord(byteOperand()) << " @ " 
               << std::setw(4) << (int)(wordAt(zeroPage(byteOperand())) + Y()) % 0x10000 << " = "
               << std::setw(2) << (int)m_memory.rawReadByte(wordAt(zeroPage(byteOperand())) + Y());
        return output.str();
    };
}

void
Cpu65XX::
buildMemoryOperationFuncs()
{
    m_readOperandFuncs[ZeroPage] = [this]() -> u8_byte {
        return m_memory.read(byteOperand());
    };
    m_readOperandFuncs[ZeroPageX] = [this]() -> u8_byte {
        return m_memory.read(zeroPage(byteOperand() + X()));
    };
    m_readOperandFuncs[ZeroPageY] = [this]() -> u8_byte {
        return m_memory.read(zeroPage(byteOperand() + Y()));
    };
    m_readOperandFuncs[Absolute] = [this]() -> u8_byte {
        return m_memory.read(wordOperand());
    };
    m_readOperandFuncs[AbsoluteX] = [this]() -> u8_byte {
        return m_memory.read(wordOperand() + X());
    };
    m_readOperandFuncs[AbsoluteY] = [this]() -> u8_byte {
        return m_memory.read(wordOperand() + Y());
    };
    m_readOperandFuncs[IndirectX] = [this]() -> u8_byte {
        return m_memory.read(wordAt(zeroPage(byteOperand() + X())));
    };
    m_readOperandFuncs[IndirectY] = [this]() -> u8_byte {
        return m_memory.read(wordAt(zeroPage(byteOperand())) + Y());
    };

    m_writeOperandFuncs[ZeroPage]   = [this](u8_byte data) -> void { 
        write(byteOperand(), data); 
    };   
    m_writeOperandFuncs[ZeroPageX] = [this](u8_byte data) -> void { 
        write(zeroPage(byteOperand() + X()), data); 
    };            
    m_writeOperandFuncs[ZeroPageY] = [this](u8_byte data) -> void { 
        write(zeroPage(byteOperand() + Y()), data); 
    };
    m_writeOperandFuncs[Absolute] = [this](u8_byte data) -> void { 
        write(wordOperand(), data); 
    };                         
    m_writeOperandFuncs[AbsoluteX] = [this](u8_byte data) -> void { 
        write(wordOperand() + X(), data); 
    };                   
    m_writeOperandFuncs[AbsoluteY] = [this](u8_byte data) -> void { 
        write(wordOperand() + Y(), data); 
    };
    m_writeOperandFuncs[IndirectX] = [this](u8_byte data) -> void { 
        write(wordAt(zeroPage(byteOperand() + X())), data); 
    };
    m_writeOperandFuncs[IndirectY] = [this](u8_byte data) -> void { 
        write(wordAt(zeroPage(byteOperand())) + Y(), data); 
    };

    m_boundaryFuncs[IndirectX] = [this]() -> bool {        
        return crossesPageBoundary(m_memory.rawReadWord(byteOperand() + X()));    
    };    
    m_boundaryFuncs[IndirectY] = [this]() -> bool {        
        return crossesPageBoundary(m_memory.rawReadWord(byteOperand()), Y());    
    };
}

void
Cpu65XX::
buildInstructionSet() {    
    // Register to Register Transfer    
    // A8        nz----  2   TAY         Transfer Accumulator to Y    Y=A    
    std::function<unsigned int()>   cycleFunc           = [this] { return 2; };    
    std::function<void ()>          workFunc            = [this] { setY(A()); };
    std::function<std::string ()>   disassemblyFunc     = m_disassemblyFunctions["None"];
    std::function<u8_byte ()>       readFunc            = m_readOperandFuncs[ZeroPage];    
    std::function<void (u8_byte)>   writeFunc           = m_writeOperandFuncs[ZeroPage];    
    auto                            boundaryFunc        = m_boundaryFuncs[IndirectX];    
    m_instructions[0xA8] = Instruction(0xA8, 1, "TAY", disassemblyFunc, cycleFunc, workFunc);    
    // AA        nz----  2   TAX         Transfer Accumulator to X    X=A    
    workFunc = [this] { setX(A()); };    
    m_instructions[0xAA] = Instruction(0xAA, 1, "TAX", disassemblyFunc, cycleFunc, workFunc);    
    // BA        nz----  2   TSX         Transfer Stack pointer to X  X=S    
    workFunc = [this] { setX(S()); };    
    m_instructions[0xBA] = Instruction(0xBA, 1, "TSX", disassemblyFunc, cycleFunc, workFunc);    
    // 98        nz----  2   TYA         Transfer Y to Accumulator    A=Y    
    workFunc = [this] { setA(Y()); };    
    m_instructions[0x98] = Instruction(0x98, 1, "TYA", disassemblyFunc, cycleFunc, workFunc);    
    // 8A        nz----  2   TXA         Transfer X to Accumulator    A=X    
    workFunc = [this] { setA(X()); };    
    m_instructions[0x8A] = Instruction(0x8A, 1, "TXA", disassemblyFunc, cycleFunc, workFunc);    
    // 9A        ------  2   TXS         Transfer X to Stack pointer  S=X      
    workFunc = [this] { setS(X()); };    
    m_instructions[0x9A] = Instruction(0x9A, 1, "TXS", disassemblyFunc, cycleFunc, workFunc);    

    // Load Register from Memory    
    // A9 nn     nz----  2   LDA #nn     Load A with Immediate     A=nn    
    workFunc = [this] { setA(byteOperand()); };    
    disassemblyFunc = m_disassemblyFunctions["Immediate"];    
    m_instructions[0xA9] = Instruction(0xA9, 2, "LDA", disassemblyFunc, cycleFunc, workFunc);    
    // A5 nn     nz----  3   LDA nn      Load A with Zero Page     A=[nn]    
    cycleFunc   = [this] { return 3; };    
    readFunc    = m_readOperandFuncs[ZeroPage];    
    workFunc    = [this, readFunc] { setA(readFunc()); };    
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];    
    m_instructions[0xA5] = Instruction(0xA5, 2, "LDA", disassemblyFunc, cycleFunc, workFunc);    
    // B5 nn     nz----  4   LDA nn,X    Load A with Zero Page,X   A=[nn+X]    
    cycleFunc   = [this] { return 4; };    
    readFunc    = m_readOperandFuncs[ZeroPageX];    
    workFunc    = [this, readFunc] { setA(readFunc()); };    
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];    
    m_instructions[0xB5] = Instruction(0xB5, 2, "LDA", disassemblyFunc, cycleFunc, workFunc);    
    // AD nn nn  nz----  4   LDA nnnn    Load A with Absolute      A=[nnnn]    
    readFunc    = m_readOperandFuncs[Absolute];    
    workFunc    = [this, readFunc] { setA(readFunc()); };     
    disassemblyFunc = m_disassemblyFunctions["AbsoluteMem"];    
    m_instructions[0xAD] = Instruction(0xAD, 3, "LDA", disassemblyFunc, cycleFunc, workFunc);    
    // BD nn nn  nz----  4*  LDA nnnn,X  Load A with Absolute,X    A=[nnnn+X]    
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), X()); };    
    readFunc    = m_readOperandFuncs[AbsoluteX];    
    workFunc    = [this, readFunc] { setA(readFunc()); };     
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];    
    m_instructions[0xBD] = Instruction(0xBD, 3, "LDA", disassemblyFunc, cycleFunc, workFunc);    
    // B9 nn nn  nz----  4*  LDA nnnn,Y  Load A with Absolute,Y    A=[nnnn+Y]    
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), Y()); };    
    readFunc    = m_readOperandFuncs[AbsoluteY];    
    workFunc    = [this, readFunc] { setA(readFunc()); };     
    disassemblyFunc = m_disassemblyFunctions["Absolute,Y"];    
    m_instructions[0xB9] = Instruction(0xB9, 3, "LDA", disassemblyFunc, cycleFunc, workFunc);    
    // A1 nn     nz----  6   LDA (nn,X)  Load A with (Indirect,X)  A=[WORD[nn+X]]    
    cycleFunc   = [this] { return 6; };    
    readFunc    = m_readOperandFuncs[IndirectX];    
    workFunc    = [this, readFunc] { setA(readFunc()); };     
    disassemblyFunc = m_disassemblyFunctions["(Indirect,X)"];   
    m_instructions[0xA1] = Instruction(0xA1, 2, "LDA", disassemblyFunc, cycleFunc, workFunc);    
    // B1 nn     nz----  5*  LDA (nn),Y  Load A with (Indirect),Y  A=[WORD[nn]+Y]    
    boundaryFunc = m_boundaryFuncs[IndirectY];    
    cycleFunc   = [this, boundaryFunc] { return 5 + boundaryFunc(); };    
    readFunc    = m_readOperandFuncs[IndirectY];    
    workFunc    = [this, readFunc] { setA(readFunc()); };    
    disassemblyFunc = m_disassemblyFunctions["(Indirect),Y"];    
    m_instructions[0xB1] = Instruction(0xB1, 2, "LDA", disassemblyFunc, cycleFunc, workFunc);    
    // A2 nn     nz----  2   LDX #nn     Load X with Immediate     X=nn    
    cycleFunc   = [this] { return 2; };     
    workFunc    = [this] { setX(byteOperand()); };    
    disassemblyFunc = m_disassemblyFunctions["Immediate"];    
    m_instructions[0xA2] = Instruction(0xA2, 2, "LDX", disassemblyFunc, cycleFunc, workFunc);    
    // A6 nn     nz----  3   LDX nn      Load X with Zero Page     X=[nn]    
    cycleFunc   = [this] { return 3; };     
    readFunc    = m_readOperandFuncs[ZeroPage];    
    workFunc    = [this, readFunc] { setX(readFunc()); };    
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];    
    m_instructions[0xA6] = Instruction(0xA6, 2, "LDX", disassemblyFunc, cycleFunc, workFunc);    
    // B6 nn     nz----  4   LDX nn,Y    Load X with Zero Page,Y   X=[nn+Y]    
    cycleFunc   = [this] { return 4; };     
    readFunc    = m_readOperandFuncs[ZeroPageY];    
    workFunc    = [this, readFunc] { setX(readFunc()); };    
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,Y"];    
    m_instructions[0xB6] = Instruction(0xB6, 2, "LDX", disassemblyFunc, cycleFunc, workFunc);    
    // AE nn nn  nz----  4   LDX nnnn    Load X with Absolute      X=[nnnn]    
    readFunc    = m_readOperandFuncs[Absolute];    
    workFunc    = [this, readFunc] { setX(readFunc()); };    
    disassemblyFunc = m_disassemblyFunctions["AbsoluteMem"];    
    m_instructions[0xAE] = Instruction(0xAE, 3, "LDX", disassemblyFunc, cycleFunc, workFunc);    
    // BE nn nn  nz----  4*  LDX nnnn,Y  Load X with Absolute,Y    X=[nnnn+Y]    
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), Y()); };     
    readFunc    = m_readOperandFuncs[AbsoluteY];    
    workFunc    = [this, readFunc] { setX(readFunc()); };    
    disassemblyFunc = m_disassemblyFunctions["Absolute,Y"];    
    m_instructions[0xBE] = Instruction(0xBE, 3, "LDX", disassemblyFunc, cycleFunc, workFunc);    
    // A0 nn     nz----  2   LDY #nn     Load Y with Immediate     Y=nn    
    cycleFunc   = [this] { return 2; };     
    workFunc    = [this] { setY(byteOperand()); };    
    disassemblyFunc = m_disassemblyFunctions["Immediate"];    
    m_instructions[0xA0] = Instruction(0xA0, 2, "LDY", disassemblyFunc, cycleFunc, workFunc);    
    // A4 nn     nz----  3   LDY nn      Load Y with Zero Page     Y=[nn]    
    cycleFunc   = [this] { return 3; };     
    readFunc    = m_readOperandFuncs[ZeroPage];    
    workFunc    = [this, readFunc] { setY(readFunc()); };    
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];    
    m_instructions[0xA4] = Instruction(0xA4, 2, "LDY", disassemblyFunc, cycleFunc, workFunc);    
    // B4 nn     nz----  4   LDY nn,X    Load Y with Zero Page,X   Y=[nn+X]    
    cycleFunc   = [this] { return 4; };     
    readFunc    = m_readOperandFuncs[ZeroPageX];    
    workFunc    = [this, readFunc] { setY(readFunc()); };    
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];    
    m_instructions[0xB4] = Instruction(0xB4, 2, "LDY", disassemblyFunc, cycleFunc, workFunc);    
    // AC nn nn  nz----  4   LDY nnnn    Load Y with Absolute      Y=[nnnn]    
    readFunc    = m_readOperandFuncs[Absolute];    
    workFunc    = [this, readFunc] { setY(readFunc()); };    
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];    
    m_instructions[0xAC] = Instruction(0xAC, 3, "LDY", disassemblyFunc, cycleFunc, workFunc);    
    // BC nn nn  nz----  4*  LDY nnnn,X  Load Y with Absolute,X    Y=[nnnn+X]    
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), X()); };    
    readFunc    = m_readOperandFuncs[AbsoluteX];    
    workFunc    = [this, readFunc] { setY(readFunc()); };    
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];    
    m_instructions[0xBC] = Instruction(0xBC, 3, "LDY", disassemblyFunc, cycleFunc, workFunc);            

    // Store Register in Memory    
    // 85 nn     ------  3   STA nn      Store A in Zero Page     [nn]=A    
    cycleFunc   = [this] { return 3; };     
    writeFunc   = m_writeOperandFuncs[ZeroPage];    
    workFunc    = [this, writeFunc] { writeFunc(A()); };    
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];    
    m_instructions[0x85] = Instruction(0x85, 2, "STA", disassemblyFunc, cycleFunc, workFunc);    
    // 95 nn     ------  4   STA nn,X    Store A in Zero Page,X   [nn+X]=A    
    cycleFunc   = [this] { return 4; };     
    writeFunc   = m_writeOperandFuncs[ZeroPageX];    
    workFunc    = [this, writeFunc] { writeFunc(A()); };    
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];    
    m_instructions[0x95] = Instruction(0x95, 2, "STA", disassemblyFunc, cycleFunc, workFunc);    
    // 8D nn nn  ------  4   STA nnnn    Store A in Absolute      [nnnn]=A    
    writeFunc   = m_writeOperandFuncs[Absolute];    
    workFunc    = [this, writeFunc] { writeFunc(A()); };    
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];    
    m_instructions[0x8D] = Instruction(0x8D, 3, "STA", disassemblyFunc, cycleFunc, workFunc);    
    // 9D nn nn  ------  5   STA nnnn,X  Store A in Absolute,X    [nnnn+X]=A    
    cycleFunc   = [this] { return 5; };     
    writeFunc   = m_writeOperandFuncs[AbsoluteX];    
    workFunc    = [this, writeFunc] { writeFunc(A()); };    
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];    
    m_instructions[0x9D] = Instruction(0x9D, 3, "STA", disassemblyFunc, cycleFunc, workFunc);    
    // 99 nn nn  ------  5   STA nnnn,Y  Store A in Absolute,Y    [nnnn+Y]=A    
    writeFunc   = m_writeOperandFuncs[AbsoluteY];    
    workFunc    = [this, writeFunc] { writeFunc(A()); };    
    disassemblyFunc = m_disassemblyFunctions["Absolute,Y"];    
    m_instructions[0x99] = Instruction(0x99, 3, "STA", disassemblyFunc, cycleFunc, workFunc);    
    // 81 nn     ------  6   STA (nn,X)  Store A in (ndirect,X)  [[nn+x]]=A    
    cycleFunc   = [this] { return 6; };     
    writeFunc   = m_writeOperandFuncs[IndirectX];    
    workFunc    = [this, writeFunc] { writeFunc(A()); };    
    disassemblyFunc = m_disassemblyFunctions["(Indirect,X)"];    
    m_instructions[0x81] = Instruction(0x81, 2, "STA", disassemblyFunc, cycleFunc, workFunc);    
    // 91 nn     ------  6   STA (nn),Y  Store A in (Indirect),Y  [[nn]+y]=A
    writeFunc   = m_writeOperandFuncs[IndirectY];
    workFunc    = [this, writeFunc] { writeFunc(A()); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect),Y"];
    m_instructions[0x91] = Instruction(0x91, 2, "STA", disassemblyFunc, cycleFunc, workFunc);
    // 86 nn     ------  3   STX nn      Store X in Zero Page     [nn]=X
    cycleFunc   = [this] { return 3; }; 
    writeFunc   = m_writeOperandFuncs[ZeroPage];
    workFunc    = [this, writeFunc] { writeFunc(X()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x86] = Instruction(0x86, 2, "STX", disassemblyFunc, cycleFunc, workFunc);
    // 96 nn     ------  4   STX nn,Y    Store X in Zero Page,Y   [nn+Y]=X
    cycleFunc   = [this] { return 4; }; 
    writeFunc   = m_writeOperandFuncs[ZeroPageY];
    workFunc    = [this, writeFunc] { writeFunc(X()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,Y"];
    m_instructions[0x96] = Instruction(0x96, 2, "STX", disassemblyFunc, cycleFunc, workFunc);
    // 8E nn nn  ------  4   STX nnnn    Store X in Absolute      [nnnn]=X
    writeFunc   = m_writeOperandFuncs[Absolute];
    workFunc    = [this, writeFunc] { writeFunc(X()); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteMem"];
    m_instructions[0x8E] = Instruction(0x8E, 3, "STX", disassemblyFunc, cycleFunc, workFunc);
    // 84 nn     ------  3   STY nn      Store Y in Zero Page     [nn]=Y
    cycleFunc   = [this] { return 3; }; 
    writeFunc   = m_writeOperandFuncs[ZeroPage];
    workFunc    = [this, writeFunc] { writeFunc(Y()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x84] = Instruction(0x84, 2, "STY", disassemblyFunc, cycleFunc, workFunc);
    // 94 nn     ------  4   STY nn,X    Store Y in Zero Page,X   [nn+X]=Y
    writeFunc   = m_writeOperandFuncs[ZeroPageX];
    cycleFunc   = [this] { return 4; }; 
    workFunc    = [this, writeFunc] { writeFunc(Y()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0x94] = Instruction(0x94, 2, "STY", disassemblyFunc, cycleFunc, workFunc);
    // 8C nn nn  ------  4   STY nnnn    Store Y in Absolute      [nnnn]=Y
    writeFunc   = m_writeOperandFuncs[Absolute];
    workFunc    = [this, writeFunc] { writeFunc(Y()); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x8C] = Instruction(0x8C, 3, "STY", disassemblyFunc, cycleFunc, workFunc);

    // Push/Pull
    // Notes: PLA sets Z and N according to content of A. The B-flag and 
    // unused flags cannot be changed by PLP, these flags are always written as "1" by PHP.
    // 48        ------  3   PHA         Push accumulator on stack        [S]=A
    cycleFunc   = [this] { return 3; }; 
    workFunc    = [this] { pushStackByte(A()); };
    disassemblyFunc = m_disassemblyFunctions["None"];
    m_instructions[0x48] = Instruction(0x48, 1, "PHA", disassemblyFunc, cycleFunc, workFunc);
    // 08        ------  3   PHP         Push processor status on stack   [S]=P
    workFunc    = [this] { 
        StatusRegister stat = StatusRegister(statusRegister());
        // The PHP opcode always write "1" into the break. 
        stat.setBreakFlag(true);
        pushStackByte(stat.value());
    };
    disassemblyFunc = m_disassemblyFunctions["None"];
    m_instructions[0x08] = Instruction(0x08, 1, "PHP", disassemblyFunc, cycleFunc, workFunc);
    // 68        nz----  4   PLA         Pull accumulator from stack      A=[S]
    cycleFunc   = [this] { return 4; }; 
    workFunc    = [this] { setA(popStackByte()); };
    disassemblyFunc = m_disassemblyFunctions["None"];
    m_instructions[0x68] = Instruction(0x68, 1, "PLA", disassemblyFunc, cycleFunc, workFunc);
    // 28        nzcidv  4   PLP         Pull processor status from stack P=[S]
    workFunc    = [this] { 
        StatusRegister stat = StatusRegister(popStackByte());
        stat.setBreakFlag(StatusRegister().breakFlag());
        setStatusRegister(stat); 
    };
    disassemblyFunc = m_disassemblyFunctions["None"];
    m_instructions[0x28] = Instruction(0x28, 1, "PLP", disassemblyFunc, cycleFunc, workFunc);

    // Add memory to accumulator with carry
    // * Add one cycle if indexing crosses a page boundary.
    // 69 nn     nzc--v  2   ADC #nn     Add Immediate           A=A+C+nn
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { setA(additionWithCarry(A(), byteOperand())); };
    disassemblyFunc = m_disassemblyFunctions["Immediate"];
    m_instructions[0x69] = Instruction(0x69, 2, "ADC", disassemblyFunc, cycleFunc, workFunc);
    // 65 nn     nzc--v  3   ADC nn      Add Zero Page           A=A+C+[nn]
    readFunc    = m_readOperandFuncs[ZeroPage];
    cycleFunc   = [this] { return 3; }; 
    workFunc    = [this, readFunc] { setA(additionWithCarry(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x65] = Instruction(0x65, 2, "ADC", disassemblyFunc, cycleFunc, workFunc);
    // 75 nn     nzc--v  4   ADC nn,X    Add Zero Page,X         A=A+C+[nn+X]
    readFunc    = m_readOperandFuncs[ZeroPageX];
    cycleFunc   = [this] { return 4; }; 
    workFunc    = [this, readFunc] { setA(additionWithCarry(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0x75] = Instruction(0x75, 2, "ADC", disassemblyFunc, cycleFunc, workFunc);
    // 6D nn nn  nzc--v  4   ADC nnnn    Add Absolute            A=A+C+[nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    workFunc    = [this, readFunc] { setA(additionWithCarry(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x6D] = Instruction(0x6D, 3, "ADC", disassemblyFunc, cycleFunc, workFunc);
    // 7D nn nn  nzc--v  4*  ADC nnnn,X  Add Absolute,X          A=A+C+[nnnn+X]
    readFunc    = m_readOperandFuncs[AbsoluteX];
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), X()); }; 
    workFunc    = [this, readFunc] { setA(additionWithCarry(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0x7D] = Instruction(0x7D, 3, "ADC", disassemblyFunc, cycleFunc, workFunc);
    // 79 nn nn  nzc--v  4*  ADC nnnn,Y  Add Absolute,Y          A=A+C+[nnnn+Y]
    readFunc    = m_readOperandFuncs[AbsoluteY];
    workFunc    = [this, readFunc] { setA(additionWithCarry(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,Y"];
    m_instructions[0x79] = Instruction(0x79, 3, "ADC", disassemblyFunc, cycleFunc, workFunc);
    // 61 nn     nzc--v  6   ADC (nn,X)  Add (Indirect,X)        A=A+C+[[nn+X]]
    readFunc    = m_readOperandFuncs[IndirectX];
    cycleFunc   = [this] { return 6; }; 
    // FIXME: Will the readFunc handle the page-end reading oddity?
    workFunc    = [this, readFunc] { setA(additionWithCarry(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect,X)"];
    m_instructions[0x61] = Instruction(0x61, 2, "ADC", disassemblyFunc, cycleFunc, workFunc);
    // 71 nn     nzc--v  5*  ADC (nn),Y  Add (Indirect),Y        A=A+C+[[nn]+Y]
    readFunc    = m_readOperandFuncs[IndirectY];
    boundaryFunc = m_boundaryFuncs[IndirectY];
    cycleFunc   = [this, boundaryFunc] { return 5 + boundaryFunc(); }; 
    workFunc    = [this, readFunc] { setA(additionWithCarry(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect),Y"];
    m_instructions[0x71] = Instruction(0x71, 2, "ADC", disassemblyFunc, cycleFunc, workFunc);

    // Subtract memory from accumulator with borrow
    // * Add one cycle if indexing crosses a page boundary.
    // FIXME: I'm not quite sure what to make of this note...
    // Note: Compared with normal 80x86 and Z80 CPUs, incoming and resulting Carry Flag are reversed.
    // E9 nn     nzc--v  2   SBC #nn     Subtract Immediate      A=A+C-1-nn
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { setA(subtractionWithBorrow(A(), byteOperand())); };
    disassemblyFunc = m_disassemblyFunctions["Immediate"];
    m_instructions[0xE9] = Instruction(0xE9, 2, "SBC", disassemblyFunc, cycleFunc, workFunc);
    // E5 nn     nzc--v  3   SBC nn      Subtract Zero Page      A=A+C-1-[nn]
    readFunc    = m_readOperandFuncs[ZeroPage];
    cycleFunc   = [this] { return 3; }; 
    workFunc    = [this, readFunc] { setA(subtractionWithBorrow(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0xE5] = Instruction(0xE5, 2, "SBC", disassemblyFunc, cycleFunc, workFunc);
    // F5 nn     nzc--v  4   SBC nn,X    Subtract Zero Page,X    A=A+C-1-[nn+X]
    readFunc    = m_readOperandFuncs[ZeroPageX];
    cycleFunc   = [this] { return 4; }; 
    workFunc    = [this, readFunc] { setA(subtractionWithBorrow(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0xF5] = Instruction(0xF5, 2, "SBC", disassemblyFunc, cycleFunc, workFunc);
    // ED nn nn  nzc--v  4   SBC nnnn    Subtract Absolute       A=A+C-1-[nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    workFunc    = [this, readFunc] { setA(subtractionWithBorrow(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0xED] = Instruction(0xED, 3, "SBC", disassemblyFunc, cycleFunc, workFunc);
    // FD nn nn  nzc--v  4*  SBC nnnn,X  Subtract Absolute,X     A=A+C-1-[nnnn+X]
    readFunc    = m_readOperandFuncs[AbsoluteX];
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), X()); }; 
    workFunc    = [this, readFunc] { setA(subtractionWithBorrow(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0xFD] = Instruction(0xFD, 3, "SBC", disassemblyFunc, cycleFunc, workFunc);
    // F9 nn nn  nzc--v  4*  SBC nnnn,Y  Subtract Absolute,Y     A=A+C-1-[nnnn+Y]
    readFunc    = m_readOperandFuncs[AbsoluteY];
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), Y()); }; 
    workFunc    = [this, readFunc] { setA(subtractionWithBorrow(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,Y"];
    m_instructions[0xF9] = Instruction(0xF9, 3, "SBC", disassemblyFunc, cycleFunc, workFunc);
    // E1 nn     nzc--v  6   SBC (nn,X)  Subtract (Indirect,X)   A=A+C-1-[[nn+X]]
    readFunc    = m_readOperandFuncs[IndirectX];
    cycleFunc   = [this] { return 6; }; 
    workFunc    = [this, readFunc] { setA(subtractionWithBorrow(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect,X)"];
    m_instructions[0xE1] = Instruction(0xE1, 2, "SBC", disassemblyFunc, cycleFunc, workFunc);
    // F1 nn     nzc--v  5*  SBC (nn),Y  Subtract (Indirect),Y   A=A+C-1-[[nn]+Y]
    readFunc    = m_readOperandFuncs[IndirectY];
    boundaryFunc = m_boundaryFuncs[IndirectY];
    cycleFunc   = [this, boundaryFunc] { return 5 + boundaryFunc(); }; 
    workFunc    = [this, readFunc] { setA(subtractionWithBorrow(A(), readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect),Y"];
    m_instructions[0xF1] = Instruction(0xF1, 2, "SBC", disassemblyFunc, cycleFunc, workFunc);

    // Logical AND memory with accumulator
    // * Add one cycle if indexing crosses a page boundary.
    // 29 nn     nz----  2   AND #nn     AND Immediate      A=A AND nn
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { setA(A() & byteOperand()); };
    disassemblyFunc = m_disassemblyFunctions["Immediate"];
    m_instructions[0x29] = Instruction(0x29, 2, "AND", disassemblyFunc, cycleFunc, workFunc);
    // 25 nn     nz----  3   AND nn      AND Zero Page      A=A AND [nn]
    cycleFunc   = [this] { return 3; }; 
    readFunc    = m_readOperandFuncs[ZeroPage];
    workFunc    = [this, readFunc] { setA(A() & readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x25] = Instruction(0x25, 2, "AND", disassemblyFunc, cycleFunc, workFunc);
    // 35 nn     nz----  4   AND nn,X    AND Zero Page,X    A=A AND [nn+X]
    readFunc    = m_readOperandFuncs[ZeroPageX];
    cycleFunc   = [this] { return 4; }; 
    workFunc    = [this, readFunc] { setA(A() & readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0x35] = Instruction(0x35, 2, "AND", disassemblyFunc, cycleFunc, workFunc);
    // 2D nn nn  nz----  4   AND nnnn    AND Absolute       A=A AND [nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    workFunc    = [this, readFunc] { setA(A() & readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x2D] = Instruction(0x2D, 3, "AND", disassemblyFunc, cycleFunc, workFunc);
    // 3D nn nn  nz----  4*  AND nnnn,X  AND Absolute,X     A=A AND [nnnn+X]
    readFunc    = m_readOperandFuncs[AbsoluteX];
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), X()); }; 
    workFunc    = [this, readFunc] { setA(A() & readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0x3D] = Instruction(0x3D, 3, "AND", disassemblyFunc, cycleFunc, workFunc);
    // 39 nn nn  nz----  4*  AND nnnn,Y  AND Absolute,Y     A=A AND [nnnn+Y]
    readFunc    = m_readOperandFuncs[AbsoluteY];
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), Y()); }; 
    workFunc    = [this, readFunc] { setA(A() & readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,Y"];
    m_instructions[0x39] = Instruction(0x39, 3, "AND", disassemblyFunc, cycleFunc, workFunc);
    // 21 nn     nz----  6   AND (nn,X)  AND (Indirect,X)   A=A AND [[nn+X]]
    readFunc    = m_readOperandFuncs[IndirectX];
    cycleFunc   = [this] { return 6; }; 
    workFunc    = [this, readFunc] { setA(A() & readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect,X)"];
    m_instructions[0x21] = Instruction(0x21, 2, "AND", disassemblyFunc, cycleFunc, workFunc);
    // 31 nn     nz----  5*  AND (nn),Y  AND (Indirect),Y   A=A AND [[nn]+Y]
    readFunc    = m_readOperandFuncs[IndirectY];
    boundaryFunc = m_boundaryFuncs[IndirectY];
    cycleFunc   = [this, boundaryFunc] { return 5 + boundaryFunc(); }; 
    workFunc    = [this, readFunc] { setA(A() & readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect),Y"];
    m_instructions[0x31] = Instruction(0x31, 2, "AND", disassemblyFunc, cycleFunc, workFunc);

    // Exclusive-OR memory with accumulator
    // * Add one cycle if indexing crosses a page boundary.
    // 49 nn     nz----  2   EOR #nn     XOR Immediate      A=A XOR nn
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { setA(A() ^ byteOperand()); };
    disassemblyFunc = m_disassemblyFunctions["Immediate"];
    m_instructions[0x49] = Instruction(0x49, 2, "EOR", disassemblyFunc, cycleFunc, workFunc);
    // 45 nn     nz----  3   EOR nn      XOR Zero Page      A=A XOR [nn]
    cycleFunc   = [this] { return 3; }; 
    readFunc    = m_readOperandFuncs[ZeroPage];
    workFunc    = [this, readFunc] { setA(A() ^ readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x45] = Instruction(0x45, 2, "EOR", disassemblyFunc, cycleFunc, workFunc);
    // 55 nn     nz----  4   EOR nn,X    XOR Zero Page,X    A=A XOR [nn+X]
    readFunc    = m_readOperandFuncs[ZeroPageX];
    workFunc    = [this, readFunc] { setA(A() ^ readFunc()); };
    cycleFunc   = [this] { return 4; }; 
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0x55] = Instruction(0x55, 2, "EOR", disassemblyFunc, cycleFunc, workFunc);
    // 4D nn nn  nz----  4   EOR nnnn    XOR Absolute       A=A XOR [nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    workFunc    = [this, readFunc] { setA(A() ^ readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x4D] = Instruction(0x4D, 3, "EOR", disassemblyFunc, cycleFunc, workFunc);
    // 5D nn nn  nz----  4*  EOR nnnn,X  XOR Absolute,X     A=A XOR [nnnn+X]
    readFunc    = m_readOperandFuncs[AbsoluteX];
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), X()); }; 
    workFunc    = [this, readFunc] { setA(A() ^ readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0x5D] = Instruction(0x5D, 3, "EOR", disassemblyFunc, cycleFunc, workFunc);
    // 59 nn nn  nz----  4*  EOR nnnn,Y  XOR Absolute,Y     A=A XOR [nnnn+Y]
    readFunc    = m_readOperandFuncs[AbsoluteY];
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), Y()); }; 
    workFunc    = [this, readFunc] { setA(A() ^ readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,Y"];
    m_instructions[0x59] = Instruction(0x59, 3, "EOR", disassemblyFunc, cycleFunc, workFunc);
    // 41 nn     nz----  6   EOR (nn,X)  XOR (Indirect,X)   A=A XOR [[nn+X]]
    readFunc    = m_readOperandFuncs[IndirectX];
    cycleFunc   = [this] { return 6; }; 
    workFunc    = [this, readFunc] { setA(A() ^ readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect,X)"];
    m_instructions[0x41] = Instruction(0x41, 2, "EOR", disassemblyFunc, cycleFunc, workFunc);
    // 51 nn     nz----  5*  EOR (nn),Y  XOR (Indirect),Y   A=A XOR [[nn]+Y]
    readFunc    = m_readOperandFuncs[IndirectY];
    boundaryFunc = m_boundaryFuncs[IndirectY];
    cycleFunc   = [this, boundaryFunc] { return 5 + boundaryFunc(); }; 
    workFunc    = [this, readFunc] { setA(A() ^ readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect),Y"];
    m_instructions[0x51] = Instruction(0x51, 2, "EOR", disassemblyFunc, cycleFunc, workFunc);

    // Logical OR memory with accumulator
    // * Add one cycle if indexing crosses a page boundary.
    // 09 nn     nz----  2   ORA #nn     OR Immediate       A=A OR nn
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { setA(A() | byteOperand()); };
    disassemblyFunc = m_disassemblyFunctions["Immediate"];
    m_instructions[0x09] = Instruction(0x09, 2, "ORA", disassemblyFunc, cycleFunc, workFunc);
    // 05 nn     nz----  3   ORA nn      OR Zero Page       A=A OR [nn]
    readFunc    = m_readOperandFuncs[ZeroPage];
    cycleFunc   = [this] { return 3; }; 
    workFunc    = [this, readFunc] { setA(A() | readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x05] = Instruction(0x05, 2, "ORA", disassemblyFunc, cycleFunc, workFunc);
    // 15 nn     nz----  4   ORA nn,X    OR Zero Page,X     A=A OR [nn+X]
    readFunc    = m_readOperandFuncs[ZeroPageX];
    cycleFunc   = [this] { return 4; }; 
    workFunc    = [this, readFunc] { setA(A() | readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0x15] = Instruction(0x15, 2, "ORA", disassemblyFunc, cycleFunc, workFunc);
    // 0D nn nn  nz----  4   ORA nnnn    OR Absolute        A=A OR [nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    workFunc    = [this, readFunc] { setA(A() | readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x0D] = Instruction(0x0D, 3, "ORA", disassemblyFunc, cycleFunc, workFunc);
    // 1D nn nn  nz----  4*  ORA nnnn,X  OR Absolute,X      A=A OR [nnnn+X]
    readFunc    = m_readOperandFuncs[AbsoluteX];
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), X()); }; 
    workFunc    = [this, readFunc] { setA(A() | readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0x1D] = Instruction(0x1D, 3, "ORA", disassemblyFunc, cycleFunc, workFunc);
    // 19 nn nn  nz----  4*  ORA nnnn,Y  OR Absolute,Y      A=A OR [nnnn+Y]
    readFunc    = m_readOperandFuncs[AbsoluteY];
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), Y()); }; 
    workFunc    = [this, readFunc] { setA(A() | readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,Y"];
    m_instructions[0x19] = Instruction(0x19, 3, "ORA", disassemblyFunc, cycleFunc, workFunc);
    // 01 nn     nz----  6   ORA (nn,X)  OR (Indirect,X)    A=A OR [[nn+X]]
    readFunc    = m_readOperandFuncs[IndirectX];
    cycleFunc   = [this] { return 6; }; 
    workFunc    = [this, readFunc] { setA(A() | readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect,X)"];
    m_instructions[0x01] = Instruction(0x01, 2, "ORA", disassemblyFunc, cycleFunc, workFunc);
    // 11 nn     nz----  5*  ORA (nn),Y  OR (Indirect),Y    A=A OR [[nn]+Y]
    readFunc    = m_readOperandFuncs[IndirectY];
    boundaryFunc = m_boundaryFuncs[IndirectY];
    cycleFunc   = [this, boundaryFunc] { return 5 + boundaryFunc(); };
    workFunc    = [this, readFunc] { setA(A() | readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect),Y"];
    m_instructions[0x11] = Instruction(0x11, 2, "ORA", disassemblyFunc, cycleFunc, workFunc);

    // Compare
    // * Add one cycle if indexing crosses a page boundary.
    //FIXME: Not sure what to make of this note... be careful!
    // Note: Compared with normal 80x86 and Z80 CPUs, resulting Carry Flag is reversed.
    // C9 nn     nzc---  2   CMP #nn     Compare A with Immediate     A-nn
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { compare(A(), byteOperand()); };
    disassemblyFunc = m_disassemblyFunctions["Immediate"];
    m_instructions[0xC9] = Instruction(0xC9, 2, "CMP", disassemblyFunc, cycleFunc, workFunc);
    // C5 nn     nzc---  3   CMP nn      Compare A with Zero Page     A-[nn]
    readFunc    = m_readOperandFuncs[ZeroPage];
    cycleFunc   = [this] { return 3; }; 
    workFunc    = [this, readFunc] { compare(A(), readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0xC5] = Instruction(0xC5, 2, "CMP", disassemblyFunc, cycleFunc, workFunc);
    // D5 nn     nzc---  4   CMP nn,X    Compare A with Zero Page,X   A-[nn+X]
    readFunc    = m_readOperandFuncs[ZeroPageX];
    cycleFunc   = [this] { return 4; }; 
    workFunc    = [this, readFunc] { compare(A(), readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0xD5] = Instruction(0xD5, 2, "CMP", disassemblyFunc, cycleFunc, workFunc);
    // CD nn nn  nzc---  4   CMP nnnn    Compare A with Absolute      A-[nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    workFunc    = [this, readFunc] { compare(A(), readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0xCD] = Instruction(0xCD, 3, "CMP", disassemblyFunc, cycleFunc, workFunc);
    // DD nn nn  nzc---  4*  CMP nnnn,X  Compare A with Absolute,X    A-[nnnn+X]
    readFunc    = m_readOperandFuncs[AbsoluteX];
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), X()); }; 
    workFunc    = [this, readFunc] { compare(A(), readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0xDD] = Instruction(0xDD, 3, "CMP", disassemblyFunc, cycleFunc, workFunc);
    // D9 nn nn  nzc---  4*  CMP nnnn,Y  Compare A with Absolute,Y    A-[nnnn+Y]
    readFunc    = m_readOperandFuncs[AbsoluteY];
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), Y()); }; 
    workFunc    = [this, readFunc] { compare(A(), readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,Y"];
    m_instructions[0xD9] = Instruction(0xD9, 3, "CMP", disassemblyFunc, cycleFunc, workFunc);
    // C1 nn     nzc---  6   CMP (nn,X)  Compare A with (Indirect,X)  A-[[nn+X]]
    readFunc    = m_readOperandFuncs[IndirectX];
    cycleFunc   = [this] { return 6; }; 
    workFunc    = [this] { compare(A(), m_memory.read(wordAt(byteOperand() + X(), true))); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect,X)"];
    m_instructions[0xC1] = Instruction(0xC1, 2, "CMP", disassemblyFunc, cycleFunc, workFunc);
    // D1 nn     nzc---  5*  CMP (nn),Y  Compare A with (Indirect),Y  A-[[nn]+Y]
    cycleFunc   = [this] { return 5 + crossesPageBoundary(wordAt(byteOperand(), true), Y()); }; 
    workFunc    = [this] { compare(A(), m_memory.read(wordAt(byteOperand(), true) + Y())); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect),Y"];
    m_instructions[0xD1] = Instruction(0xD1, 2, "CMP", disassemblyFunc, cycleFunc, workFunc);
    // E0 nn     nzc---  2   CPX #nn     Compare X with Immediate     X-nn
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { compare(X(), byteOperand()); };
    disassemblyFunc = m_disassemblyFunctions["Immediate"];
    m_instructions[0xE0] = Instruction(0xE0, 2, "CPX", disassemblyFunc, cycleFunc, workFunc);
    // E4 nn     nzc---  3   CPX nn      Compare X with Zero Page     X-[nn]
    readFunc    = m_readOperandFuncs[ZeroPage];
    cycleFunc   = [this] { return 3; }; 
    workFunc    = [this, readFunc] { compare(X(), readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0xE4] = Instruction(0xE4, 2, "CPX", disassemblyFunc, cycleFunc, workFunc);
    // EC nn nn  nzc---  4   CPX nnnn    Compare X with Absolute      X-[nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    cycleFunc   = [this] { return 4; }; 
    workFunc    = [this, readFunc] { compare(X(), readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0xEC] = Instruction(0xEC, 3, "CPX", disassemblyFunc, cycleFunc, workFunc);
    // C0 nn     nzc---  2   CPY #nn     Compare Y with Immediate     Y-nn
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { compare(Y(), byteOperand()); };
    disassemblyFunc = m_disassemblyFunctions["Immediate"];
    m_instructions[0xC0] = Instruction(0xC0, 2, "CPY", disassemblyFunc, cycleFunc, workFunc);
    // C4 nn     nzc---  3   CPY nn      Compare Y with Zero Page     Y-[nn]
    readFunc    = m_readOperandFuncs[ZeroPage];
    cycleFunc   = [this] { return 3; }; 
    workFunc    = [this, readFunc] { compare(Y(), readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0xC4] = Instruction(0xC4, 2, "CPY", disassemblyFunc, cycleFunc, workFunc);
    // CC nn nn  nzc---  4   CPY nnnn    Compare Y with Absolute      Y-[nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    cycleFunc   = [this] { return 4; }; 
    workFunc    = [this, readFunc] { compare(Y(), readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0xCC] = Instruction(0xCC, 3, "CPY", disassemblyFunc, cycleFunc, workFunc);

    // Bit Test
    // 24 nn     xz---x  3   BIT nn      Bit Test   A AND [nn], N=[nn].7, V=[nn].6
    cycleFunc   = [this] { return 3; }; 
    readFunc    = m_readOperandFuncs[ZeroPage];
    workFunc    = [this, readFunc] { bitTest(A(), readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x24] = Instruction(0x24, 2, "BIT", disassemblyFunc, cycleFunc, workFunc);
    // 2C nn nn  xz---x  4   BIT nnnn    Bit Test   A AND [..], N=[..].7, V=[..].6
    cycleFunc   = [this] { return 4; }; 
    readFunc    = m_readOperandFuncs[Absolute];
    workFunc    = [this, readFunc] { bitTest(A(), readFunc()); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x2C] = Instruction(0x2C, 3, "BIT", disassemblyFunc, cycleFunc, workFunc);

    // Increment by one
    // E6 nn     nz----  5   INC nn      Increment Zero Page    [nn]=[nn]+1
    cycleFunc   = [this] { return 5; }; 
    readFunc    = m_readOperandFuncs[ZeroPage];
    writeFunc   = m_writeOperandFuncs[ZeroPage];
    workFunc    = [this, writeFunc, readFunc] { writeFunc(increment(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0xE6] = Instruction(0xE6, 2, "INC", disassemblyFunc, cycleFunc, workFunc);
    // F6 nn     nz----  6   INC nn,X    Increment Zero Page,X  [nn+X]=[nn+X]+1
    cycleFunc   = [this] { return 6; }; 
    readFunc    = m_readOperandFuncs[ZeroPageX];
    writeFunc   = m_writeOperandFuncs[ZeroPageX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(increment(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0xF6] = Instruction(0xF6, 2, "INC", disassemblyFunc, cycleFunc, workFunc);
    // EE nn nn  nz----  6   INC nnnn    Increment Absolute     [nnnn]=[nnnn]+1
    cycleFunc   = [this] { return 6; }; 
    readFunc    = m_readOperandFuncs[Absolute];
    writeFunc   = m_writeOperandFuncs[Absolute];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(increment(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0xEE] = Instruction(0xEE, 3, "INC", disassemblyFunc, cycleFunc, workFunc);
    // FE nn nn  nz----  7   INC nnnn,X  Increment Absolute,X   [nnnn+X]=[nnnn+X]+1
    cycleFunc   = [this] { return 7; }; 
    readFunc    = m_readOperandFuncs[AbsoluteX];
    writeFunc   = m_writeOperandFuncs[AbsoluteX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(increment(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0xFE] = Instruction(0xFE, 3, "INC", disassemblyFunc, cycleFunc, workFunc);
    // E8        nz----  2   INX         Increment X            X=X+1
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { setX(X() + 1); };
    disassemblyFunc = m_disassemblyFunctions["None"];
    m_instructions[0xE8] = Instruction(0xE8, 1, "INX", disassemblyFunc, cycleFunc, workFunc);
    // C8        nz----  2   INY         Increment Y            Y=Y+1
    workFunc    = [this] { setY(Y() + 1); };
    m_instructions[0xC8] = Instruction(0xC8, 1, "INY", disassemblyFunc, cycleFunc, workFunc);

    // Decrement by one
    // C6 nn     nz----  5   DEC nn      Decrement Zero Page    [nn]=[nn]-1
    cycleFunc   = [this] { return 5; }; 
    readFunc    = m_readOperandFuncs[ZeroPage];
    writeFunc   = m_writeOperandFuncs[ZeroPage];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(decrement(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0xC6] = Instruction(0xC6, 2, "DEC", disassemblyFunc, cycleFunc, workFunc);
    // D6 nn     nz----  6   DEC nn,X    Decrement Zero Page,X  [nn+X]=[nn+X]-1
    cycleFunc   = [this] { return 6; }; 
    readFunc    = m_readOperandFuncs[ZeroPageX];
    writeFunc   = m_writeOperandFuncs[ZeroPageX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(decrement(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0xD6] = Instruction(0xD6, 2, "DEC", disassemblyFunc, cycleFunc, workFunc);
    // CE nn nn  nz----  6   DEC nnnn    Decrement Absolute     [nnnn]=[nnnn]-1
    readFunc    = m_readOperandFuncs[Absolute];
    writeFunc   = m_writeOperandFuncs[Absolute];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(decrement(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0xCE] = Instruction(0xCE, 3, "DEC", disassemblyFunc, cycleFunc, workFunc);
    // DE nn nn  nz----  7   DEC nnnn,X  Decrement Absolute,X   [nnnn+X]=[nnnn+X]-1
    cycleFunc   = [this] { return 7; }; 
    readFunc    = m_readOperandFuncs[AbsoluteX];
    writeFunc   = m_writeOperandFuncs[AbsoluteX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(decrement(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0xDE] = Instruction(0xDE, 3, "DEC", disassemblyFunc, cycleFunc, workFunc);
    // CA        nz----  2   DEX         Decrement X            X=X-1
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { setX(X() - 1); };
    disassemblyFunc = m_disassemblyFunctions["None"];
    m_instructions[0xCA] = Instruction(0xCA, 1, "DEX", disassemblyFunc, cycleFunc, workFunc);
    // 88        nz----  2   DEY         Decrement Y            Y=Y-1
    workFunc    = [this] { setY(Y() - 1); };
    disassemblyFunc = m_disassemblyFunctions["None"];
    m_instructions[0x88] = Instruction(0x88, 1, "DEY", disassemblyFunc, cycleFunc, workFunc);

    //Shift Left
    // 0A        nzc---  2   ASL A       Shift Left Accumulator   SHL A
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { setA(shiftLeft(A())); };
    disassemblyFunc = m_disassemblyFunctions["A"];
    m_instructions[0x0A] = Instruction(0x0A, 1, "ASL", disassemblyFunc, cycleFunc, workFunc);
    // 06 nn     nzc---  5   ASL nn      Shift Left Zero Page     SHL [nn]
    cycleFunc   = [this] { return 5; }; 
    readFunc    = m_readOperandFuncs[ZeroPage];
    writeFunc   = m_writeOperandFuncs[ZeroPage];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(shiftLeft(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x06] = Instruction(0x06, 2, "ASL", disassemblyFunc, cycleFunc, workFunc);
    // 16 nn     nzc---  6   ASL nn,X    Shift Left Zero Page,X   SHL [nn+X]
    cycleFunc   = [this] { return 6; }; 
    readFunc    = m_readOperandFuncs[ZeroPageX];
    writeFunc   = m_writeOperandFuncs[ZeroPageX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(shiftLeft(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0x16] = Instruction(0x16, 2, "ASL", disassemblyFunc, cycleFunc, workFunc);
    // 0E nn nn  nzc---  6   ASL nnnn    Shift Left Absolute      SHL [nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    writeFunc   = m_writeOperandFuncs[Absolute];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(shiftLeft(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x0E] = Instruction(0x0E, 3, "ASL", disassemblyFunc, cycleFunc, workFunc);
    // 1E nn nn  nzc---  7   ASL nnnn,X  Shift Left Absolute,X    SHL [nnnn+X]
    cycleFunc   = [this] { return 7; }; 
    readFunc    = m_readOperandFuncs[AbsoluteX];
    writeFunc   = m_writeOperandFuncs[AbsoluteX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(shiftLeft(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0x1E] = Instruction(0x1E, 3, "ASL", disassemblyFunc, cycleFunc, workFunc);

    // Shift Right
    // 4A        0zc---  2   LSR A       Shift Right Accumulator  SHR A
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { setA(shiftRight(A())); };
    disassemblyFunc = m_disassemblyFunctions["A"];
    m_instructions[0x4A] = Instruction(0x4A, 1, "LSR", disassemblyFunc, cycleFunc, workFunc);
    // 46 nn     0zc---  5   LSR nn      Shift Right Zero Page    SHR [nn]
    cycleFunc   = [this] { return 5; }; 
    readFunc    = m_readOperandFuncs[ZeroPage];
    writeFunc   = m_writeOperandFuncs[ZeroPage];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(shiftRight(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x46] = Instruction(0x46, 2, "LSR", disassemblyFunc, cycleFunc, workFunc);
    // 56 nn     0zc---  6   LSR nn,X    Shift Right Zero Page,X  SHR [nn+X]
    cycleFunc   = [this] { return 6; }; 
    readFunc    = m_readOperandFuncs[ZeroPageX];
    writeFunc   = m_writeOperandFuncs[ZeroPageX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(shiftRight(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0x56] = Instruction(0x56, 2, "LSR", disassemblyFunc, cycleFunc, workFunc);
    // 4E nn nn  0zc---  6   LSR nnnn    Shift Right Absolute     SHR [nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    writeFunc   = m_writeOperandFuncs[Absolute];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(shiftRight(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x4E] = Instruction(0x4E, 3, "LSR", disassemblyFunc, cycleFunc, workFunc);
    // 5E nn nn  0zc---  7   LSR nnnn,X  Shift Right Absolute,X   SHR [nnnn+X]
    cycleFunc   = [this] { return 7; }; 
    readFunc    = m_readOperandFuncs[AbsoluteX];
    writeFunc   = m_writeOperandFuncs[AbsoluteX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(shiftRight(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0x5E] = Instruction(0x5E, 3, "LSR", disassemblyFunc, cycleFunc, workFunc);

    // Rotate Left through Carry
    // 2A        nzc---  2   ROL A       Rotate Left Accumulator  RCL A
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { setA(rotateLeftThroughCarry(A())); };
    disassemblyFunc = m_disassemblyFunctions["A"];
    m_instructions[0x2A] = Instruction(0x2A, 1, "ROL", disassemblyFunc, cycleFunc, workFunc);
    // 26 nn     nzc---  5   ROL nn      Rotate Left Zero Page    RCL [nn]
    cycleFunc   = [this] { return 5; }; 
    readFunc    = m_readOperandFuncs[ZeroPage];
    writeFunc   = m_writeOperandFuncs[ZeroPage];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(rotateLeftThroughCarry(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x26] = Instruction(0x26, 2, "ROL", disassemblyFunc, cycleFunc, workFunc);
    // 36 nn     nzc---  6   ROL nn,X    Rotate Left Zero Page,X  RCL [nn+X]
    cycleFunc   = [this] { return 6; }; 
    readFunc    = m_readOperandFuncs[ZeroPageX];
    writeFunc   = m_writeOperandFuncs[ZeroPageX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(rotateLeftThroughCarry(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0x36] = Instruction(0x36, 2, "ROL", disassemblyFunc, cycleFunc, workFunc);
    // 2E nn nn  nzc---  6   ROL nnnn    Rotate Left Absolute     RCL [nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    writeFunc   = m_writeOperandFuncs[Absolute];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(rotateLeftThroughCarry(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x2E] = Instruction(0x2E, 3, "ROL", disassemblyFunc, cycleFunc, workFunc);
    // 3E nn nn  nzc---  7   ROL nnnn,X  Rotate Left Absolute,X   RCL [nnnn+X]
    cycleFunc   = [this] { return 7; }; 
    readFunc    = m_readOperandFuncs[AbsoluteX];
    writeFunc   = m_writeOperandFuncs[AbsoluteX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(rotateLeftThroughCarry(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0x3E] = Instruction(0x3E, 3, "ROL", disassemblyFunc, cycleFunc, workFunc);

    // Rotate Right through Carry
    // 6A        nzc---  2   ROR A       Rotate Right Accumulator RCR A
    cycleFunc   = [this] { return 2; }; 
    workFunc    = [this] { setA(rotateRightThroughCarry(A())); };
    disassemblyFunc = m_disassemblyFunctions["A"];
    m_instructions[0x6A] = Instruction(0x6A, 1, "ROR", disassemblyFunc, cycleFunc, workFunc);
    // 66 nn     nzc---  5   ROR nn      Rotate Right Zero Page   RCR [nn]
    cycleFunc   = [this] { return 5; }; 
    readFunc    = m_readOperandFuncs[ZeroPage];
    writeFunc   = m_writeOperandFuncs[ZeroPage];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(rotateRightThroughCarry(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x66] = Instruction(0x66, 2, "ROR", disassemblyFunc, cycleFunc, workFunc);
    // 76 nn     nzc---  6   ROR nn,X    Rotate Right Zero Page,X RCR [nn+X]
    cycleFunc   = [this] { return 6; }; 
    readFunc    = m_readOperandFuncs[ZeroPageX];
    writeFunc   = m_writeOperandFuncs[ZeroPageX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(rotateRightThroughCarry(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    m_instructions[0x76] = Instruction(0x76, 2, "ROR", disassemblyFunc, cycleFunc, workFunc);
    // 6E nn nn  nzc---  6   ROR nnnn    Rotate Right Absolute    RCR [nnnn]
    cycleFunc   = [this] { return 6; }; 
    readFunc    = m_readOperandFuncs[Absolute];
    writeFunc   = m_writeOperandFuncs[Absolute];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(rotateRightThroughCarry(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x6E] = Instruction(0x6E, 3, "ROR", disassemblyFunc, cycleFunc, workFunc);
    // 7E nn nn  nzc---  7   ROR nnnn,X  Rotate Right Absolute,X  RCR [nnnn+X]
    cycleFunc   = [this] { return 7; }; 
    readFunc    = m_readOperandFuncs[AbsoluteX];
    writeFunc   = m_writeOperandFuncs[AbsoluteX];
    workFunc    = [this, readFunc, writeFunc] { writeFunc(rotateRightThroughCarry(readFunc())); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0x7E] = Instruction(0x7E, 3, "ROR", disassemblyFunc, cycleFunc, workFunc);

    // Normal Jumps
    // 4C nn nn  ------  3   JMP nnnn    Jump Absolute              PC=nnnn
    cycleFunc   = [this] { return 3; }; 
    workFunc    = [this] { setPC(wordOperand()); };
    disassemblyFunc = m_disassemblyFunctions["Absolute"];
    m_instructions[0x4C] = Instruction(0x4C, 3, "JMP", disassemblyFunc, cycleFunc, workFunc);
    // Glitch: For JMP [nnnn] the operand word cannot cross page boundaries, ie. JMP [03FFh] would 
    // fetch the MSB from [0300h] instead of [0400h]. Very simple workaround would be to place a ALIGN 2 before the data word.
    // 6C nn nn  ------  5   JMP (nnnn)  Jump Indirect              PC=WORD[nnnn]
    cycleFunc   = [this] { return 5; }; 
    // Indirect TRUE
    workFunc    = [this] { setPC(wordAt(wordOperand(), true)); };
    disassemblyFunc = m_disassemblyFunctions["Indirect"];
    m_instructions[0x6C] = Instruction(0x6C, 3, "JMP", disassemblyFunc, cycleFunc, workFunc);
    // 20 nn nn  ------  6   JSR nnnn    Jump and Save Return Addr. [S]=PC+2,PC=nnnn
    cycleFunc   = [this] { return 6; }; 
    workFunc    = [this] { 
        pushStackWord(PC() + 2); 
        setPC(wordOperand());
    };
    disassemblyFunc = m_disassemblyFunctions["Absolute"];
    m_instructions[0x20] = Instruction(0x20, 3, "JSR", disassemblyFunc, cycleFunc, workFunc);
    // Note: RTI cannot modify the B-Flag or the unused flag.
    // 40        nzcidv  6   RTI         Return from BRK/IRQ/NMI    P=[S], PC=[S]
    cycleFunc   = [this] { return 6; }; 
    workFunc    = [this] { 
        // Save break flag.
        bool breakFlag = m_status.breakFlag();
        setStatusRegister(StatusRegister(popStackByte()));
        // Restore break flag.
        m_status.setBreakFlag(breakFlag);
        setPC(popStackWord());
    };
    disassemblyFunc = m_disassemblyFunctions["None"];
    m_instructions[0x40] = Instruction(0x40, 1, "RTI", disassemblyFunc, cycleFunc, workFunc);
    // 60        ------  6   RTS         Return from Subroutine     PC=[S]+1
    cycleFunc   = [this] { return 6; }; 
    workFunc    = [this] { setPC(popStackWord() + 1); };
    m_instructions[0x60] = Instruction(0x60, 1, "RTS", disassemblyFunc, cycleFunc, workFunc);

    // Conditional Branches
    // ** The execution time is 2 cycles if the condition is false (no branch executed). Otherwise, 
    // 3 cycles if the destination is in the same memory page, or 4 cycles if it crosses a page boundary (see below for exact info).
    // Note: After subtractions (SBC or CMP) carry=set indicates above-or-equal, unlike as for 80x86 and Z80 CPUs. 
    // Obviously, this still applies even when using 80XX-style syntax.

    // Conditional Branch Page Crossing
    // The branch opcode with parameter takes up two bytes, causing the PC to get incremented twice (PC=PC+2), 
    // without any extra boundary cycle. The signed parameter is then added to the PC (PC+disp), the extra clock 
    // cycle occurs if the addition crosses a page boundary (next or previous 100h-page).
    auto conditionalBranchCycleFunc = [this](bool condition, const u8_byte& offset){
        if (condition) {
            return 3 + conditionalBranchCrossesPageBoundary(offset);
        }
        return 2;
    };

    // 10 dd     ------  2** BPL disp    Branch on result plus     if N=0 PC=PC+/-nn
    cycleFunc   = [=, this] { return conditionalBranchCycleFunc(!m_status.negative(), byteOperand()); };
    workFunc    = [this] { conditionalBranch(!m_status.negative(), byteOperand()); };
    disassemblyFunc = m_disassemblyFunctions["Branch"];
    m_instructions[0x10] = Instruction(0x10, 2, "BPL", disassemblyFunc, cycleFunc, workFunc);
    // 30 dd     ------  2** BMI disp    Branch on result minus    if N=1 PC=PC+/-nn
    cycleFunc   = [=, this] { return conditionalBranchCycleFunc(m_status.negative(), byteOperand()); }; 
    workFunc    = [this] { conditionalBranch(m_status.negative(), byteOperand()); };
    m_instructions[0x30] = Instruction(0x30, 2, "BMI", disassemblyFunc, cycleFunc, workFunc);
    // 50 dd     ------  2** BVC disp    Branch on overflow clear  if V=0 PC=PC+/-nn
    cycleFunc   = [=, this] { return conditionalBranchCycleFunc(!m_status.overflow(), byteOperand()); };
    workFunc    = [this] { conditionalBranch(!m_status.overflow(), byteOperand()); };
    m_instructions[0x50] = Instruction(0x50, 2, "BVC", disassemblyFunc, cycleFunc, workFunc);
    // 70 dd     ------  2** BVS disp    Branch on overflow set    if V=1 PC=PC+/-nn
    cycleFunc   = [=, this] { return conditionalBranchCycleFunc(m_status.overflow(), byteOperand()); };
    workFunc    = [this] { conditionalBranch(m_status.overflow(), byteOperand()); };
    m_instructions[0x70] = Instruction(0x70, 2, "BVS", disassemblyFunc, cycleFunc, workFunc);
    // 90 dd     ------  2** BCC disp    Branch on carry clear     if C=0 PC=PC+/-nn
    cycleFunc   = [=, this] { return conditionalBranchCycleFunc(!m_status.carry(), byteOperand()); };
    workFunc    = [this] { conditionalBranch(!m_status.carry(), byteOperand()); };
    m_instructions[0x90] = Instruction(0x90, 2, "BCC", disassemblyFunc, cycleFunc, workFunc);
    // B0 dd     ------  2** BCS disp    Branch on carry set       if C=1 PC=PC+/-nn
    cycleFunc   = [=, this] { return conditionalBranchCycleFunc(m_status.carry(), byteOperand()); };
    workFunc    = [this] { conditionalBranch(m_status.carry(), byteOperand()); };
    m_instructions[0xB0] = Instruction(0xB0, 2, "BCS", disassemblyFunc, cycleFunc, workFunc);
    // D0 dd     ------  2** BNE disp    Branch on result not zero if Z=0 PC=PC+/-nn
    cycleFunc   = [=, this] { return conditionalBranchCycleFunc(!m_status.zero(), byteOperand()); };
    workFunc    = [this] { conditionalBranch(!m_status.zero(), byteOperand()); };
    m_instructions[0xD0] = Instruction(0xD0, 2, "BNE", disassemblyFunc, cycleFunc, workFunc);
    // F0 dd     ------  2** BEQ disp    Branch on result zero     if Z=1 PC=PC+/-nn
    cycleFunc   = [=, this] { return conditionalBranchCycleFunc(m_status.zero(), byteOperand()); };
    workFunc    = [this] { conditionalBranch(m_status.zero(), byteOperand()); };
    m_instructions[0xF0] = Instruction(0xF0, 2, "BEQ", disassemblyFunc, cycleFunc, workFunc);

    // Interrupts, Exceptions, Breakpoints
    // 00        ---1--  7   BRK   Force Break B=1 [S]=PC+1,[S]=P,I=1,PC=[FFFE]
    cycleFunc   = [this] { return 7; };
    workFunc    = [this] { 
            m_status.setBreakFlag(true);
            pushStackWord(PC() + 1);
            pushStackByte(m_status.value());
            m_status.setIRQDisable(true);
            setPC(wordAt(0xFFFE));
    };
    disassemblyFunc = m_disassemblyFunctions["None"];
    m_instructions[0x00] = Instruction(0x00, 1, "BRK", disassemblyFunc, cycleFunc, workFunc);

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
    cycleFunc   = [this] { return 2; };
    workFunc    = [this] { m_status.setCarry(false); };
    disassemblyFunc = m_disassemblyFunctions["None"];
    m_instructions[0x18] = Instruction(0x18, 1, "CLC", disassemblyFunc, cycleFunc, workFunc);
    // 58        ---0--  2   CLI         Clear interrupt disable bit I=0
    cycleFunc   = [this] { return 2; };
    workFunc    = [this] { m_status.setIRQDisable(false); };
    m_instructions[0x58] = Instruction(0x58, 1, "CLI", disassemblyFunc, cycleFunc, workFunc);
    // D8        ----0-  2   CLD         Clear decimal mode          D=0
    cycleFunc   = [this] { return 2; };
    workFunc    = [this] { m_status.setDecimalMode(false); };
    m_instructions[0xD8] = Instruction(0xD8, 1, "CLD", disassemblyFunc, cycleFunc, workFunc);
    // B8        -----0  2   CLV         Clear overflow flag         V=0
    cycleFunc   = [this] { return 2; };
    workFunc    = [this] { m_status.setOverflow(false); };
    m_instructions[0xB8] = Instruction(0xB8, 1, "CLV", disassemblyFunc, cycleFunc, workFunc);
    // 38        --1---  2   SEC         Set carry flag              C=1
    cycleFunc   = [this] { return 2; };
    workFunc    = [this] { m_status.setCarry(true); };
    m_instructions[0x38] = Instruction(0x38, 1, "SEC", disassemblyFunc, cycleFunc, workFunc);
    // 78        ---1--  2   SEI         Set interrupt disable bit   I=1
    cycleFunc   = [this] { return 2; };
    workFunc    = [this] { m_status.setIRQDisable(true); };
    m_instructions[0x78] = Instruction(0x78, 1, "SEI", disassemblyFunc, cycleFunc, workFunc);
    // F8        ----1-  2   SED         Set decimal mode            D=1
    cycleFunc   = [this] { return 2; };
    workFunc    = [this] { m_status.setDecimalMode(true); };
    m_instructions[0xF8] = Instruction(0xF8, 1, "SED", disassemblyFunc, cycleFunc, workFunc);

    // No Operation
    // EA        ------  2   NOP         No operation                No operation
    cycleFunc   = [this] { return 2; };
    workFunc    = [this] {};
    m_instructions[0xEA] = Instruction(0xEA, 1, "NOP", disassemblyFunc, cycleFunc, workFunc);
        
    // 'Illegal' opcodes. 
    // Not sure how wide-spread the use of these instructions are, but since the publicly available
    // nestest.rom exercises them, I'll support them for completeness' sake.

    // SAX and LAX

    // 87 nn     ------  3   SAX nn      STA+STX  [nn]=A AND X
    cycleFunc   = [this] { return 3; };
    writeFunc   = m_writeOperandFuncs[ZeroPage];
    workFunc    = [this, writeFunc] { writeFunc(A() & X()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0x87] = Instruction(0x87, 2, "SAX", disassemblyFunc, cycleFunc, workFunc);
    // 97 nn     ------  4   SAX nn,Y    STA+STX  [nn+Y]=A AND X
    cycleFunc   = [this] { return 4; };
    writeFunc   = m_writeOperandFuncs[ZeroPageY];
    workFunc    = [this, writeFunc] { writeFunc(A() & X()); };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,Y"];
    m_instructions[0x97] = Instruction(0x97, 2, "SAX", disassemblyFunc, cycleFunc, workFunc);
    // 8F nn nn  ------  4   SAX nnnn    STA+STX  [nnnn]=A AND X
    writeFunc   = m_writeOperandFuncs[Absolute];
    workFunc    = [this, writeFunc] { writeFunc(A() & X()); };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x8F] = Instruction(0x8F, 3, "SAX", disassemblyFunc, cycleFunc, workFunc);
    // 83 nn     ------  6   SAX (nn,X)  STA+STX  [WORD[nn+X]]=A AND X
    cycleFunc   = [this] { return 6; };
    writeFunc   = m_writeOperandFuncs[IndirectX];
    workFunc    = [this, writeFunc] { writeFunc(A() & X()); };
    disassemblyFunc = m_disassemblyFunctions["(Indirect,X)"];
    m_instructions[0x83] = Instruction(0x83, 2, "SAX", disassemblyFunc, cycleFunc, workFunc);

    // A7 nn     nz----  3   LAX nn      LDA+LDX  A,X=[nn]
    cycleFunc   = [this] { return 3; };
    readFunc    = m_readOperandFuncs[ZeroPage];
    workFunc    = [this, readFunc] { 
        u8_byte data = readFunc();
        setA(data);
        setX(data);
    };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    m_instructions[0xA7] = Instruction(0xA7, 2, "LAX", disassemblyFunc, cycleFunc, workFunc);
    // B7 nn     nz----  4   LAX nn,Y    LDA+LDX  A,X=[nn+Y]
    cycleFunc   = [this] { return 4; };
    readFunc    = m_readOperandFuncs[ZeroPageY];
    workFunc    = [this, readFunc] { 
        u8_byte data = readFunc();
        setA(data);
        setX(data);
    };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,Y"];
    m_instructions[0xB7] = Instruction(0xB7, 2, "LAX", disassemblyFunc, cycleFunc, workFunc);
    // AF nn nn  nz----  4   LAX nnnn    LDA+LDX  A,X=[nnnn]
    readFunc    = m_readOperandFuncs[Absolute];
    workFunc    = [this, readFunc] { 
        u8_byte data = readFunc();
        setA(data);
        setX(data);
    };
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0xAF] = Instruction(0xAF, 3, "LAX", disassemblyFunc, cycleFunc, workFunc);
    // BF nn nn  nz----  4*  LAX nnnn,X  LDA+LDX  A,X=[nnnn+X]
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), X()); };
    // Yes, we're reading using AbsoluteY addressing mode, no it doesn't make any sense.
    readFunc    = m_readOperandFuncs[AbsoluteY];
    workFunc    = [this, readFunc] { 
        u8_byte data = readFunc();
        setA(data);
        setX(data);
    };
    disassemblyFunc = m_disassemblyFunctions["Absolute,Y"];
    m_instructions[0xBF] = Instruction(0xBF, 3, "LAX", disassemblyFunc, cycleFunc, workFunc);
    // A3 nn     nz----  6   LAX (nn,X)  LDA+LDX  A,X=[WORD[nn+X]]
    cycleFunc   = [this] { return 6; };
    readFunc    = m_readOperandFuncs[IndirectX];
    workFunc    = [this, readFunc] { 
        u8_byte data = readFunc();
        setA(data);
        setX(data);
    };
    disassemblyFunc = m_disassemblyFunctions["(Indirect,X)"];
    m_instructions[0xA3] = Instruction(0xA3, 2, "LAX", disassemblyFunc, cycleFunc, workFunc);
    // B3 nn     nz----  5*  LAX (nn),Y  LDA+LDX  A,X=[WORD[nn]+Y]
    boundaryFunc = m_boundaryFuncs[IndirectY];
    cycleFunc   = [this, boundaryFunc] { return 5 + boundaryFunc(); };
    readFunc    = m_readOperandFuncs[IndirectY];
    workFunc    = [this, readFunc] { 
        u8_byte data = readFunc();
        setA(data);
        setX(data);
    };
    disassemblyFunc = m_disassemblyFunctions["(Indirect),Y"];
    m_instructions[0xB3] = Instruction(0xB3, 2, "LAX", disassemblyFunc, cycleFunc, workFunc);

    buildCombinedALUOpcodes();
    
    // Other Illegal Opcodes

    // 0B nn     nzc---  2  ANC #nn          AND+ASL  A=A AND nn
    cycleFunc               = [this] { return 2; };
    workFunc                = [this] { setA(A() & byteOperand()); };
    disassemblyFunc         = m_disassemblyFunctions["Immediate"];
    m_instructions[0x0B]    = Instruction(0x0B, 2, "ANC", disassemblyFunc, cycleFunc, workFunc);

    // 2B nn     nzc---  2  ANC #nn          AND+ROL  A=A AND nn
    workFunc                = [this] { setA(A() & byteOperand()); };
    disassemblyFunc         = m_disassemblyFunctions["Immediate"];
    m_instructions[0x2B]    = Instruction(0x2B, 2, "ANC", disassemblyFunc, cycleFunc, workFunc);

    // 4B nn     nzc---  2  ALR #nn          AND+LSR  A=(A AND nn)*2  MUL2???
    workFunc                = [this] { setA(shiftRight(A() & byteOperand())); };
    disassemblyFunc         = m_disassemblyFunctions["Immediate"];
    m_instructions[0x4B]    = Instruction(0x4B, 2, "ALR", disassemblyFunc, cycleFunc, workFunc);

    // 6B nn     nzc--v  2  ARR #nn          AND+ROR  A=(A AND nn)/2
    workFunc                = [this] { setA(rotateRightThroughCarry(A() & byteOperand())); };
    disassemblyFunc         = m_disassemblyFunctions["Immediate"];
    m_instructions[0x6B]    = Instruction(0x6B, 2, "ARR", disassemblyFunc, cycleFunc, workFunc);

    // 8B nn     nz----  2  XAA #nn    ((2)) TXA+AND  A=X AND nn
    // note to XAA: DO NOT USE!!! Highly unstable!!!
    workFunc                = [this] { setA(X() & byteOperand()); };
    disassemblyFunc         = m_disassemblyFunctions["Immediate"];
    m_instructions[0x8B]    = Instruction(0x8B, 2, "XAA", disassemblyFunc, cycleFunc, workFunc);

    // AB nn     nz----  2  LAX #nn    ((2)) LDA+TAX  A,X=nn
    // note to LAX: DO NOT USE!!! On my C128, this opcode is stable, but on my C64-II
    // it loses bits so that the operation looks like this: ORA #? AND #{imm} TAX.
    workFunc                = [this] { 
        setX(A() & byteOperand()); 
        setA(X());
    };
    disassemblyFunc         = m_disassemblyFunctions["Immediate"];
    m_instructions[0xAB]    = Instruction(0xAB, 2, "ANC", disassemblyFunc, cycleFunc, workFunc);

    // CB nn     nzc---  2  AXS #nn          CMP+DEX  X=A AND X -nn
    // note to AXS: performs CMP and DEX at the same time, so that the MINUS sets
    // the flag like CMP, not SBC.
    workFunc                = [this] { setX((A() & X()) - byteOperand()); };
    disassemblyFunc         = m_disassemblyFunctions["Immediate"];
    m_instructions[0xCB]    = Instruction(0xCB, 2, "AXS", disassemblyFunc, cycleFunc, workFunc);

    // EB nn     nzc--v  2  SBC #nn          SBC+NOP  A=A-nn         cy?
    workFunc                = [this] { setA(subtractionWithBorrow(A(), byteOperand())); };
    disassemblyFunc         = m_disassemblyFunctions["Immediate"];
    m_instructions[0xEB]    = Instruction(0xEB, 2, "SBC", disassemblyFunc, cycleFunc, workFunc);

    // BB nn nn  nz----  4* LAS nnnn,Y       LDA+TSX  A,X,S = [nnnn+Y] AND S
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), Y()); };
    readFunc    = m_readOperandFuncs[AbsoluteY];
    workFunc    = [this, readFunc] { 
        setA(readFunc() & S()); 
        setX(A());
        setS(A());
    };
    disassemblyFunc         = m_disassemblyFunctions["Absolute,Y"];
    m_instructions[0xBB]    = Instruction(0xBB, 3, "LAS", disassemblyFunc, cycleFunc, workFunc);

    // 93 nn     ------  6  AHX (nn),Y ((1))          [WORD[nn]+Y] = A AND X AND H
    cycleFunc               = [this] { return 6; };
    workFunc                = [this] { 
        u16_word address = wordAt(byteOperand()) + Y();
        u8_byte h = (address & 0xFF00) >> 8;
        write(m_memory.read(address), A() & X() & h ); 
    };
    disassemblyFunc         = m_disassemblyFunctions["Immediate"];
    m_instructions[0x93]    = Instruction(0x93, 2, "AHX", disassemblyFunc, cycleFunc, workFunc);

    // note: sometimes the &H drops off. Also page boundary crossing will not work as
    //       expected (the bank where the value is stored may be equal to the value stored).
    //       ["H" probably meant to be the MSB aka Highbyte of the 16bit memory address?]

    // 9F nn nn  ------  5  AHX nnnn,Y ((1))          [nnnn+Y] = A AND X AND H
    // AHX {adr} = stores A&X&H into {adr}
    cycleFunc               = [this] { return 5; };
    workFunc                = [this] { 
        u16_word address = wordOperand() + Y();
        u8_byte h = (address & 0xFF00) >> 8;
        write(m_memory.read(address), A() & X() & h);  
    };
    disassemblyFunc         = m_disassemblyFunctions["Absolute,Y"];
    m_instructions[0x9F]    = Instruction(0x9F, 3, "AHX", disassemblyFunc, cycleFunc, workFunc);

    // 9C nn nn  ------  5  SHY nnnn,X ((1))          [nnnn+X] = Y AND H
    // SHY {adr} = stores Y&H into {adr}
    workFunc                = [this] { 
        u16_word address = wordOperand() + X();
        u8_byte h = (address & 0xFF00) >> 8;
        write(m_memory.read(address), Y() & h);  
    };
    disassemblyFunc         = m_disassemblyFunctions["Absolute,X"];
    m_instructions[0x9C]    = Instruction(0x9C, 3, "SHY", disassemblyFunc, cycleFunc, workFunc);

    // 9E nn nn  ------  5  SHX nnnn,Y ((1))          [nnnn+Y] = X AND H
    // SHX {adr} = stores X&H into {adr}
    workFunc                = [this] { 
        u16_word address = wordOperand() + Y();
        u8_byte h = (address & 0xFF00) >> 8;
        write(m_memory.read(address), X() & h);  
    };
    disassemblyFunc         = m_disassemblyFunctions["Absolute,Y"];
    m_instructions[0x9E]    = Instruction(0x9E, 3, "SHX", disassemblyFunc, cycleFunc, workFunc);

    // 9B nn nn  ------  5  TAS nnnn,Y ((1)) STA+TXS  S=A AND X  // [nnnn+Y]=S AND H
    workFunc                = [this] { 
        u16_word address = wordOperand() + Y();
        u8_byte h = (address & 0xFF00) >> 8;
        setS(A() & X()); 
        write(m_memory.read(address), S() & h);
    };
    disassemblyFunc         = m_disassemblyFunctions["Absolute,Y"];
    m_instructions[0x9B]    = Instruction(0x9B, 3, "TAS", disassemblyFunc, cycleFunc, workFunc);

    // NUL/NOP and KIL/JAM/HLT

    // xx        ------  2   NOP        (xx=1A,3A,5A,7A,DA,FA)
    cycleFunc   = [this] { return 2; };
    workFunc    = [this] {};
    disassemblyFunc = m_disassemblyFunctions["None"];
    for (const u8_byte& opcode : {0x1A, 0x3A, 0x5A, 0x7A, 0xDA, 0xFA}) {
        m_instructions[opcode] = Instruction(opcode, 1, "NOP", disassemblyFunc, cycleFunc, workFunc);
    }
    // xx nn     ------  2   NOP #nn    (xx=80,82,89,C2,E2)
    cycleFunc   = [this] { return 2; };
    disassemblyFunc = m_disassemblyFunctions["Immediate"];
    for (const u8_byte& opcode : {0x80, 0x82, 0x89, 0xC2, 0xE2}) {
        m_instructions[opcode] = Instruction(opcode, 2, "NOP", disassemblyFunc, cycleFunc, workFunc);
    }
    // xx nn     ------  3   NOP nn     (xx=04,44,64)
    cycleFunc   = [this] { return 3; };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage"];
    for (const u8_byte& opcode : {0x04, 0x44, 0x64}) {
        m_instructions[opcode] = Instruction(opcode, 2, "NOP", disassemblyFunc, cycleFunc, workFunc);
    }
    // xx nn     ------  4   NOP nn,X   (xx=14,34,54,74,D4,F4)
    cycleFunc   = [this] { return 4; };
    disassemblyFunc = m_disassemblyFunctions["ZeroPage,X"];
    for (const u8_byte& opcode : {0x14, 0x34, 0x54, 0x74, 0xD4, 0xF4}) {
        m_instructions[opcode] = Instruction(opcode, 2, "NOP", disassemblyFunc, cycleFunc, workFunc);
    }
    // xx nn nn  ------  4   NOP nnnn   (xx=0C)
    disassemblyFunc = m_disassemblyFunctions["AbsoluteWithValue"];
    m_instructions[0x0C] = Instruction(0x0C, 3, "NOP", disassemblyFunc, cycleFunc, workFunc);
    // xx nn nn  ------  4*  NOP nnnn,X (xx=1C,3C,5C,7C,DC,FC)
    cycleFunc   = [this] { return 4 + crossesPageBoundary(wordOperand(), X()); };
    disassemblyFunc = m_disassemblyFunctions["Absolute,X"];
    for (const u8_byte& opcode : {0x1C, 0x3C, 0x5C, 0x7C, 0xDC, 0xFC}) {
        m_instructions[opcode] = Instruction(opcode, 3, "NOP", disassemblyFunc, cycleFunc, workFunc);
    }
    // xx        ------  -   KIL        (xx=02,12,22,32,42,52,62,72,92,B2,D2,F2)
    cycleFunc   = [this] { return 0; };
    disassemblyFunc = m_disassemblyFunctions["None"];
    for (const u8_byte& opcode : {0x02, 0x12, 0x22, 0x32, 0x42, 0x52, 0x62, 0x72, 0x92, 0xB2, 0xD2, 0xF2}) {
        m_instructions[opcode] = Instruction(opcode, 3, "NOP", disassemblyFunc, cycleFunc, workFunc);
    }

    // Add all the 'illegal' opcodes to an easy to query set.
    for (const u8_byte& opcode : 
            { 0x87, 0x97, 0x8F, 0x83, 0xA7,
              0xB7, 0xAF, 0xBF, 0xA3, 0xB3,
              0x0B, 0x2B, 0x4B, 0x6B, 0x8B,
              0xAB, 0xCB, 0xEB, 0x93, 0x9F,
              0x9C, 0x9E, 0x9B, 0xBB, 0x1A, 
              0x3A, 0x5A, 0x7A, 0xDA, 0xFA,
              0x80, 0x82, 0x89, 0xC2, 0xE2,
              0x04, 0x44, 0x64, 0x14, 0x34,
              0x54, 0x74, 0xD4, 0xF4, 0x0C,
              0x1C, 0x3C, 0x5C, 0x7C, 0xDC,
              0xFC, 0x02, 0x12, 0x22, 0x32,
              0x42, 0x52, 0x62, 0x72, 0x92,
              0xB2, 0xD2, 0xF2 }
        ) {
        m_illegalInstructions.insert(opcode);
    }
}

void 
Cpu65XX::
buildCombinedALUOpcodes()  
{
    // Combined ALU-Opcodes
    // Opcode high-bits, flags, commands:

    // Opcode low-bits, clock cycles, operands:

    // 07+xx nn        5    nn       [nn]
    // 17+xx nn        6    nn,X     [nn+X]
    // 03+xx nn        8    (nn,X)   [WORD[nn+X]]
    // 13+xx nn        8    (nn),Y   [WORD[nn]+Y]
    // 0F+xx nn nn     6    nnnn     [nnnn]
    // 1F+xx nn nn     7    nnnn,X   [nnnn+X]
    // 1B+xx nn nn     7    nnnn,Y   [nnnn+Y]

    // Selects addressing mode.

    std::function<void ()> workFunc;

    const u8_byte lowbitsArray[] = { 0x07, 0x17, 0x03, 0x13, 0x0F, 0x1F, 0x1B, 0x00 };

    std::function<u8_byte()> readFuncs[] = {
        m_readOperandFuncs[ZeroPage],
        m_readOperandFuncs[ZeroPageX],
        m_readOperandFuncs[IndirectX],
        m_readOperandFuncs[IndirectY],
        m_readOperandFuncs[Absolute],
        m_readOperandFuncs[AbsoluteX],
        m_readOperandFuncs[AbsoluteY]
    };

    std::function<void(u8_byte)> writeFuncs[] = {
        m_writeOperandFuncs[ZeroPage],
        m_writeOperandFuncs[ZeroPageX],
        m_writeOperandFuncs[IndirectX],
        m_writeOperandFuncs[IndirectY],
        m_writeOperandFuncs[Absolute],
        m_writeOperandFuncs[AbsoluteX],
        m_writeOperandFuncs[AbsoluteY]
    };

    unsigned int lengths[] = {
        2,
        2,
        2,
        2,
        3,
        3,
        3
    };

    std::function<unsigned int()> cycleFuncs[] = {
        std::function<unsigned int()>([] { return 5; }),
        std::function<unsigned int()>([] { return 6; }),
        std::function<unsigned int()>([] { return 8; }),
        std::function<unsigned int()>([] { return 8; }),
        std::function<unsigned int()>([] { return 6; }),
        std::function<unsigned int()>([] { return 7; }),
        std::function<unsigned int()>([] { return 7; })
    };

    std::vector<std::function<std::string()>> disassemblyFuncs = {
        m_disassemblyFunctions["ZeroPage"],
        m_disassemblyFunctions["ZeroPage,X"],
        m_disassemblyFunctions["(Indirect,X)"],
        m_disassemblyFunctions["(Indirect),Y"],
        m_disassemblyFunctions["AbsoluteWithValue"],
        m_disassemblyFunctions["Absolute,X"],
        m_disassemblyFunctions["Absolute,Y"],
    };

    // Build each instruction using the high half of the byte to select the work.
    for (const u8_byte highbits : { 0x00, 0x20, 0x40, 0x60, 0xC0, 0xE0 }) {
        // The low half selects how the operand is fetched.
        for (int i = 0; lowbitsArray[i] != 0x00; ++i) {
            u8_byte lowbits                         = lowbitsArray[i];
            auto readFunc                           = readFuncs[i];
            auto writeFunc                          = writeFuncs[i];
            const char* mnemonic                    = NULL;

            switch (highbits) {
                // 00+yy        nzc---  SLO op   ASL+ORA   op=op SHL 1 // A=A OR op
                case 0x00:
                    workFunc = [this, readFunc, writeFunc] { 
                        u8_byte operand = readFunc();
                        operand = shiftLeft(operand);
                        writeFunc(operand);
                        setA(A() | operand); 
                    };
                    mnemonic = "SLO";
                    break;
                // 20+yy        nzc---  RLA op   ROL+AND   op=op RCL 1 // A=A AND op
                case 0x20:
                    workFunc = [this, readFunc, writeFunc] { 
                        u8_byte operand = readFunc();
                        operand = rotateLeftThroughCarry(operand);
                        writeFunc(operand);
                        setA(A() & operand); 
                    };
                    mnemonic = "RLA";
                    break;
                // 40+yy        nzc---  SRE op   LSR+EOR   op=op SHR 1 // A=A XOR op
                case 0x40:
                    workFunc = [this, readFunc, writeFunc] { 
                        u8_byte operand = readFunc();
                        operand = shiftRight(operand);
                        writeFunc(operand);
                        setA(A() ^ operand); 
                    };
                    mnemonic = "SRE";
                    break;
                // 60+yy        nzc--v  RRA op   ROR+ADC   op=op RCR 1 // A=A ADC op
                case 0x60:
                    workFunc = [this, readFunc, writeFunc] { 
                        u8_byte operand = readFunc(); 
                        operand = rotateRightThroughCarry(operand);
                        writeFunc(operand);
                        setA(additionWithCarry(A(), operand));
                    };
                    mnemonic = "RRA";
                    break;
                // C0+yy        nzc---  DCP op   DEC+CMP   op=op-1     // A-op
                case 0xC0:
                    workFunc = [this, readFunc, writeFunc] { 
                        u8_byte operand = readFunc(); 
                        operand = decrement(operand);
                        writeFunc(operand);
                        compare(A(), operand);
                    };
                    mnemonic = "DCP";
                    break;
                // E0+yy        nzc--v  ISC op   INC+SBC   op=op+1     // A=A-op-(1-cy)
                case 0xE0:
                    workFunc = [this, readFunc, writeFunc] { 
                        u8_byte operand = readFunc();
                        operand = increment(operand);
                        writeFunc(operand);
                        setA(subtractionWithBorrow(A(), operand)); 
                    };
                    mnemonic = "ISB";
                    break;
                default:
                    std::cout << "Got bad highbits when constructing Combined ALU opcodes!\n";
                    mnemonic = "ILL";
            }

            // Builds the instruction.
            u8_byte opcode = highbits | lowbits;
            // Remember, this instruction is technically illegal!
            m_illegalInstructions.insert(opcode);
            m_instructions[opcode] = Instruction(opcode, lengths[i], mnemonic, disassemblyFuncs[i], cycleFuncs[i], workFunc);
        }
    }
}


void
Cpu65XX::
tick()
{
    // Handle NMIs
    // --        ---1--  ??  /NMI  NMI         B=0 [S]=PC,[S]=P,I=1,PC=[FFFA]
    if (m_NMI) {
        // Main NMI work.
        m_status.setBreakFlag(false);
        pushStackWord(PC());
        pushStackWord(statusRegister().value());
        m_status.setIRQDisable(true);
        setPC(0xFFFA);
        // Clean up processor state.
        m_downCycles = 0;
        m_queuedInstruction = NULL;
        m_NMI = false;
    } 

    // Handle IRQs
    // FIXME: Not sure if this is correct... refer to statement below:
    // If /IRQ is kept LOW then same (old) interrupt is executed again as soon as setting I=0.
    if (m_IRQ &&
        !m_status.IRQDisable()) {
        // Main IRQ work.
        m_status.setBreakFlag(false);
        pushStackWord(PC());
        pushStackWord(statusRegister().value());
        m_status.setIRQDisable(true);
        setPC(0xFFFE);
        // Clean up processor state.
        m_downCycles = 0;
        m_queuedInstruction = NULL;
        m_IRQ = false;
    }

    // If we have cycles to burn, then burn them.
    if (m_downCycles) {
        m_downCycles--;
        m_cycles++;
        return;
    }

    // m_downCycles == 0
    // Execute queued instruction
    if (m_queuedInstruction) {
        u16_word lastPC = PC();
        m_queuedInstruction->apply();
        // TODO: This seems a bit hacky, as I feel each instruction should
        // decide for itself what needs to happen to the PC.
        if (PC() == lastPC) {
            setPC(PC() + m_queuedInstruction->length());
        }
    }

    // Fetch and queue the next instruction.
    m_queuedInstruction = &(m_instructions[m_memory.read(PC())]);

    m_lastInstructionDebugOut = debugOutput();

    // Determine how long to wait until we execute it.
    m_downCycles = m_queuedInstruction->cycles();
}

const std::string&
Cpu65XX::
lastInstructionDebugOut() const
{
    return m_lastInstructionDebugOut;
}

void
Cpu65XX::
signalNMI() 
{
    m_NMI = true;
}

// Cpu65XX accessors
const u8_byte &
Cpu65XX::
A() const
{
    return m_A;
}

const u8_byte &
Cpu65XX::
X() const
{
    return m_X;
}

const u8_byte &
Cpu65XX::
Y() const
{
    return m_Y;
}

const u16_word &
Cpu65XX::
PC() const
{
    return m_PC;
}

const u8_byte &
Cpu65XX::
S() const
{
    return m_S;
}

u16_word
Cpu65XX::
stackPointer() const
{
    return 0x0100 + static_cast<u16_word>(S());
}

const Cpu65XX::StatusRegister&
Cpu65XX::
statusRegister() const
{
    return m_status;
}

unsigned int
Cpu65XX::
cycles() const
{
    return m_cycles;
}

unsigned int
Cpu65XX::
downCycles() const
{
    return m_downCycles;
}

u8_byte           
Cpu65XX::
byteOperand() 
{
    return m_memory.rawReadByte(PC() + 1);
}

u16_word          
Cpu65XX::
wordOperand() 
{
    // return m_memory.rawReadWord(PC() + 1);
    return wordAt(PC() + 1);
}

bool
Cpu65XX::
crossesPageBoundary(const u16_word& address) const
{ 
    return (address & 0x00FF) == 0xFF; 
}

bool
Cpu65XX::
crossesPageBoundary(const u16_word& baseAddress, const u8_byte& offset) const
{
    u16_word finalAddress = baseAddress + offset;
    return ((finalAddress & 0xFF00) != (baseAddress & 0xFF00));
}

bool
Cpu65XX::
conditionalBranchCrossesPageBoundary(const u8_byte& offset) const
{
    u16_word destination = PC() + 2 + static_cast<signed char>(offset);
    return (destination & 0xFF00) != ((PC() + 2) & 0xFF00);
}

std::string
Cpu65XX::
debugOutput() 
{
    if (nullptr == m_queuedInstruction) { return std::string(); }

    std::stringstream output;
    output.fill('0');
    output << std::hex << std::setw(4) << m_PC << "  ";
    unsigned int length = m_queuedInstruction->length();
    for (unsigned int i = 0; i < length; ++i) {
        output << std::setw(2) << (int)m_memory.rawReadByte(m_PC+i) << " ";
    }
    
    bool illegalInstruction = m_illegalInstructions.find(m_memory.rawReadByte(m_PC)) != m_illegalInstructions.end();
    int spaces = 7 - ((length - 1) * 3) - illegalInstruction;
    output << std::string(spaces, ' ');
    if (illegalInstruction) {
        output << '*';
    }
    output << m_queuedInstruction->mnemonic();
    std::string disassembly = m_queuedInstruction->disassembly();
    output << " " << disassembly;
    output << std::string(28 - disassembly.length(), ' ');
    output << "A:"   << std::setw(2) << (int)A() 
           << " X:"  << std::setw(2) << (int)X() 
           << " Y:"  << std::setw(2) << (int)Y() 
           << " P:"  << std::setw(2) << (int)m_status.value()
           << " SP:" << std::setw(2) << (int)S();
    output.fill(' ');
    output << std::dec << " CYC:" << std::setw(3) << ((m_cycles*3) % 341)
           << " SL:" << (short)(((242 + ((m_cycles*3)/341)) % 262) - 1) << std::endl;

    // output << statusRegisterState();
           
    std::string upperCased = output.str();
    std::transform(upperCased.begin(), upperCased.end(), upperCased.begin(), &toupper );
    return upperCased;
}

std::string
Cpu65XX::
statusRegisterState() const
{
    std::stringstream output;
    output << "NV-BDIZC" << std::endl
        << m_status.negative() << m_status.overflow() << m_status.unusedFlag() << m_status.breakFlag()
        << m_status.decimalMode() << m_status.IRQDisable() << m_status.zero() 
        << m_status.carry() << std::endl;
    return output.str();
}

std::string
Cpu65XX::
state() const 
{
    std::stringstream output;
    output << std::hex << "A:  " << static_cast<u16_word>(m_A) 
           << " X:  " << static_cast<u16_word>(m_X)  
           << " Y:  " << static_cast<u16_word>(m_Y)  
           << " PC: " << static_cast<u16_word>(m_PC) 
           << " S:  " << static_cast<u16_word>(m_S)  
           << " P:  " << static_cast<u16_word>(m_status.value()) << std::endl
           << "CZIDBON" << std::endl
           << m_status.carry() << m_status.zero() << m_status.IRQDisable() << m_status.decimalMode()
           << m_status.breakFlag() << m_status.overflow() << m_status.negative() << std::endl;

    return output.str();
}

// Cpu65XX mutators
void
Cpu65XX::
setA(u8_byte value) 
{
    handleRegisterAssignmentFlags(value);
    m_A = value;
}

void
Cpu65XX::
setX(u8_byte value) 
{
    handleRegisterAssignmentFlags(value);
    m_X = value;
}

void
Cpu65XX::
setY(u8_byte value) 
{
    handleRegisterAssignmentFlags(value);
    m_Y = value;
}

void
Cpu65XX::
setPC(const u16_word& value) 
{
    m_PC = value;
}

void
Cpu65XX::
setS(u8_byte value) 
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
handleRegisterAssignmentFlags(u8_byte value) 
{
    // If the value is zero, then the zero flag is set.
    // If the value is not zero, then the zero flag is zero, go figure!
    m_status.setZero(0x00 == value);
    // The highest bit determines whether the value is negative or not.
    m_status.setNegative(value & 0x80);
}

u8_byte 
Cpu65XX::
additionWithCarry(const u8_byte& op1, const u8_byte& op2)
{
    //FIXME: Surely we can make this more efficient...
    u16_word result = static_cast<u16_word>(op1) + static_cast<u8_byte>(m_status.carry()) + op2;
    m_status.setCarry(result > 0xFF);
    m_status.setOverflow((A() ^ result) & (op2 ^ result) & 0x80);
    return static_cast<u8_byte>(result);
}

u8_byte 
Cpu65XX::
subtractionWithBorrow(const u8_byte& op1, const u8_byte& op2)
{
    u16_word result = op1 - op2 - (1 - m_status.carry());
    m_status.setOverflow(((A() ^ result) & 0x80) && ((A()^ op2) & 0x80));
    m_status.setCarry(op1 >= op2);
    return static_cast<u8_byte>(result);
}

void          
Cpu65XX::
compare(const u8_byte& accumulator, const u8_byte& memory)
{
    u8_byte result = accumulator - memory;
    m_status.setZero(result == 0);
    m_status.setCarry(accumulator >= memory);
    m_status.setNegative(result & 0x80);
}

u8_byte
Cpu65XX::
increment(u8_byte value)
{
    u8_byte result = value + 1;
    m_status.setZero(result == 0);
    m_status.setNegative(result & 0x80);
    return result;
}

u8_byte
Cpu65XX::
decrement(u8_byte value)
{
    u8_byte result = value - 1;
    m_status.setZero(result == 0);
    m_status.setNegative(result & 0x80);
    return result;
}

u8_byte 
Cpu65XX::
shiftLeft(const u8_byte& op)
{
    u8_byte result = op << 1;
    m_status.setZero(result == 0);
    m_status.setNegative(result & 0x80);
    m_status.setCarry(op & 0x80);
    return result;
}

u8_byte 
Cpu65XX::
shiftRight(const u8_byte& op)
{
    u8_byte result = op >> 1;
    m_status.setNegative(false);
    m_status.setZero(result);
    m_status.setCarry(op & 0x01);
    return result;
}

u8_byte 
Cpu65XX::
rotateLeftThroughCarry(const u8_byte& op)
{
    u8_byte result = (op << 1) + m_status.carry();
    m_status.setCarry(op & 0x80);
    m_status.setZero(result == 0);
    m_status.setNegative(result & 0x80);
    return result;
}

u8_byte 
Cpu65XX::
rotateRightThroughCarry(const u8_byte& operand)
{
    u8_byte result = (operand >> 1) + (m_status.carry() * 0x80);
    m_status.setCarry(operand & 0x01);
    m_status.setZero(result == 0);
    m_status.setNegative(result & 0x80);
    return result;
}

void
Cpu65XX::
write(Memory::address_t destination, u8_byte value)
{
    m_memory.write(destination, value);
}

void
Cpu65XX::
bitTest(const u8_byte& op1, const u8_byte& op2) 
{
    u8_byte result = op1 & op2;
    m_status.setNegative(op2 & 0x80);
    m_status.setOverflow(op2 & 0x40);
    m_status.setZero(result == 0);
}

void
Cpu65XX::
conditionalBranch(bool condition, const u8_byte& destination)
{
    if (condition)
    {
        setPC(PC() + 2 + static_cast<signed char>(destination));
    }
    else
    {
        setPC(PC() + 2);
    }
}

void 
Cpu65XX::
pushStackByte(const u8_byte& value) 
{
    m_memory.rawWrite(stackPointer(), value);
    setS(S() - 1);
}

void 
Cpu65XX::
pushStackWord(const u16_word& value)
{
    m_memory.rawWrite(stackPointer(), (u8_byte)((value & 0xFF00) >> 8));
    m_memory.rawWrite(stackPointer() - 1, (u8_byte)value);
    setS(S() - 2);
}

u8_byte
Cpu65XX::
popStackByte()
{
    setS(S() + 1);
    return m_memory.rawReadByte(stackPointer());
}

u16_word 
Cpu65XX::
popStackWord()
{
    setS(S() + 2);
    u16_word value = (((u16_word)m_memory.rawReadByte(stackPointer())) << 8) + m_memory.rawReadByte(stackPointer() - 1);
    return value;
}

void
Cpu65XX::
resetImpl()
{
    //TODO...
}

void
Cpu65XX::
powerOnImpl()
{
    //TODO...
}

void
Cpu65XX::
powerOffImpl()
{
    //TODO...
}

//FIXME: Consider what to do when the address is out-of-bounds.
// Throw an expection? 

// This routine correctly maps addresses to account for the memory mirroring
// found in the CPU's memory map.
#if 0
u16_word
Cpu65XX::Memory::
trueAddress(const u16_word& address) const
{
    u16_word fixedAddress = address;
    // The following three ranges are mirrors of 0x0000 - 0x07FF : 
    // 0x0800 - 0x0FFF 
    // 0x1000 - 0x17FF
    // 0x1800 - 0x1FFF
    if (address >= 0x0800 &&
        address <  0x2000) {
        fixedAddress = address % 0x0800;
    }
    // 0x2000 - 0x2007 Are the PPU registers
    // 0x2000 - 0x3FFF Are the same PPU registers mapped over and over. Seems silly...
    // TODO: Need to route memory access through PPU/APU/etc registers.
    else if (address >= 0x2008 && 
             address <  0x4000) {
        fixedAddress = 0x2000 + (address % 0x0008);
    }
    return fixedAddress;
}
#endif

u16_word
Cpu65XX::
wordAt(Memory::address_t address, bool indirect) 
{
    // Zero page indirect 
    // (If the address is in the zero page OR we're an indirect read) AND
    // (The address is at the top of the page)
    // Then return the word consisting of the bottom (0xXX00) byte of the page, and the last byte (0xXXFF) of the page.
    if ((address < 0x0100 || indirect) && (address & 0x00FF) == 0xFF) {
        return m_memory.rawReadByte(address & 0xFF00) * 0x0100 + 
               m_memory.rawReadByte(address);
    }
    return m_memory.rawReadWord(address);
}

// Begin Instruction implementation
Cpu65XX::Instruction::
Instruction() :
    m_opcode (0xEA),
    m_length (1),
    m_disassemblyFunction ([] { return std::string(); }),
    m_cycleFunction ([] { return 2; }),
    m_workFunction  ([] {}),
    m_mnemonic      ("ILL")
{
}

Cpu65XX::Instruction::
Instruction(
        u8_byte opcode,
        u8_byte length,
        const char * mnemonic,
        std::function<std::string()>& disassemblyFunc,
        std::function<unsigned int()>& cycleFunc,
        std::function<void ()>& workFunc
) : m_opcode (opcode),
    m_length (length),
    m_mnemonic (mnemonic),
    m_disassemblyFunction (disassemblyFunc),
    m_cycleFunction (cycleFunc),
    m_workFunction (workFunc)
{
}

const u8_byte&      
Cpu65XX::Instruction::
opcode()    const
{
    return m_opcode;
}

const u8_byte& 
Cpu65XX::Instruction::
length()    const
{
    return m_length;
}

unsigned int       
Cpu65XX::Instruction::
cycles()    const 
{
    return m_cycleFunction();
}

const std::string&  
Cpu65XX::Instruction::
mnemonic()  const
{
    return m_mnemonic;
}

void                
Cpu65XX::Instruction::
apply()     const
{
    m_workFunction();
}

std::string
Cpu65XX::Instruction::
disassembly() const
{
    return m_disassemblyFunction();
}

// End Instruction Implementation

// Begin StatusRegister Implementation
Cpu65XX::StatusRegister::
StatusRegister() 
{
    // IRQ disable flag seems to be 1 on startup...
    m_status[2] = 1;
    // This flag is not used but always 1.
    m_status[5] = 1;
}


Cpu65XX::StatusRegister::
StatusRegister(u8_byte value) 
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

bool
Cpu65XX::StatusRegister::
unusedFlag() const
{
    return m_status[5];
}

u8_byte
Cpu65XX::StatusRegister::
value() const
{
    return static_cast<u8_byte>(m_status.to_ulong());
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



