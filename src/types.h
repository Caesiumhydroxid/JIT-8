// Copyright 2021 Betamark Pty Ltd. All rights reserved.
// Author: Shlomi Nissan (shlomi@betamark.com)

#ifndef CHIP8_TYPES_H
#define CHIP8_TYPES_H

#include <cinttypes>
#include <sstream>
#include <array>

namespace c8 {

struct Opcode {
    uint16_t in;

    Opcode(uint16_t opcode): in(opcode) {}

    [[nodiscard]] uint8_t x() const { return (in & 0x0F00) >> 8; }
    [[nodiscard]] uint8_t y() const { return (in & 0x00F0) >> 4; }
    [[nodiscard]] uint8_t byte() const { return in & 0x00FF; }
    [[nodiscard]] uint16_t address() const { return in & 0x0FFF; }
    [[nodiscard]] uint16_t high() const { return in >> 12; }
    [[nodiscard]] uint16_t low() const { return in & 0x000F; }
};

}

#endif  // CHIP8_TYPES_H