#pragma once
#include <cinttypes>
#include <SFML/Graphics.hpp>
#include <time.h>
#include "display.h"

class Hardware{
    public:
        static const int REG_F = 15;
        static const int AMOUNT_REGISTERS = 16;
        static const int STACK_SIZE = 16;
        static const int AMOUNT_BUTTONS = 16;
        uint64_t slowdown = 0;

        uint8_t regs[AMOUNT_REGISTERS] = {0};
        uint16_t stack[STACK_SIZE] = {0};
        uint8_t buttons[AMOUNT_BUTTONS] = {0};
        
        volatile uint16_t indexRegister = 0;
        volatile uint16_t pc = 0;
        volatile uint32_t rngstate = time(NULL);
        volatile uint16_t sp = 0;

        volatile uint8_t soundTimer = 0;
        volatile uint8_t delayTimer = 0;

        Display display;

        void setKeyState(sf::Keyboard::Key k, bool state);
        void printState();
};
