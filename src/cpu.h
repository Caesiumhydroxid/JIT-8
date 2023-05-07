#ifndef _CPU_H_
#define _CPU_H_
#include <cinttypes>
#include "display.h"

class CPU{
    public:
        static const int REG_F = 15;
        static const int AMOUNT_REGISTERS = 16;
        static const int STACK_SIZE = 16;

        uint8_t regs[AMOUNT_REGISTERS] = {0};
        uint16_t stack[STACK_SIZE] = {0};

        
        volatile uint16_t indexRegister = 0;
        volatile uint16_t pc = 0;
        volatile uint32_t rng = 1;
        volatile uint16_t sp = 0;

        volatile uint8_t soundTimer = 0;
        volatile uint8_t delayTimer = 0;

        Display display;
        void printState();
};

#endif // !_CPU_H_