#include "cpu.h"
#include <iostream>

void CPU::printState()
{
    for(int i=0;i<AMOUNT_REGISTERS;i++)
    {
        std::cout << "Reg[" << std::hex << i << "]= " << std::hex << unsigned(regs[i]) << " ";
        if(i % 8 == 7){
            std::cout << std::endl;
        }
    }
    std::cout << "Reg[I] "  << std::hex << indexRegister << " " ;
    std::cout << "Reg[SP] " << std::hex << sp << " ";
    std::cout << "Reg[PC] " << std::hex << pc << std::endl;

    for(int i=0;i<STACK_SIZE;i++)
    {
        std::cout << std::hex << unsigned(stack[i]) << " ";
        if(i % 8 == 7){
            std::cout << std::endl;
        }
    }
}