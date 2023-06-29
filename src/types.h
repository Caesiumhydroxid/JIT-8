#pragma once
#include <cinttypes>
#include <sstream>
#include <array>


struct Opcode {
    uint16_t in;
    Opcode(uint16_t opcode): in(opcode) {}
    uint8_t x() const { return (in & 0x0F00) >> 8; }
    uint8_t y() const { return (in & 0x00F0) >> 4; }
    uint8_t kk() const { return in & 0x00FF; }
    uint16_t nnn() const { return in & 0x0FFF; }
    uint16_t high() const { return in >> 12; }
    uint16_t n() const { return in & 0x000F; }
};