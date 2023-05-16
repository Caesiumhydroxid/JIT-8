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

void CPU::setKeyState(sf::Keyboard::Key k, bool state)
{
    switch(k){
        case sf::Keyboard::Num1: buttons[0x1]=state; break;
        case sf::Keyboard::Num2: buttons[0x2]=state; break;
        case sf::Keyboard::Num3: buttons[0x3]=state; break;
        case sf::Keyboard::Num4: buttons[0xC]=state; break;
        case sf::Keyboard::Q:    buttons[0x4]=state; break;
        case sf::Keyboard::W:    buttons[0x5]=state; break;
        case sf::Keyboard::E:    buttons[0x6]=state; break;
        case sf::Keyboard::R:    buttons[0xD]=state; break;
        case sf::Keyboard::A:    buttons[0x7]=state; break;
        case sf::Keyboard::S:    buttons[0x8]=state; break;
        case sf::Keyboard::D:    buttons[0x9]=state; break;
        case sf::Keyboard::F:    buttons[0xE]=state; break;
        case sf::Keyboard::Y:    buttons[0xA]=state; break;
        case sf::Keyboard::X:    buttons[0x0]=state; break;
        case sf::Keyboard::C:    buttons[0xB]=state; break;
        case sf::Keyboard::V:    buttons[0xF]=state; break;
    }
}