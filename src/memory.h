// Copyright 2021 Betamark Pty Ltd. All rights reserved.
// Author: Shlomi Nissan (shlomi@betamark.com)

#ifndef CHIP8_MEMORY_H
#define CHIP8_MEMORY_H

#include <array>
#include <cinttypes>

typedef uint64_t (*BasicBlockFunction)(void);
class BasicBlock;

namespace c8 {


class Memory {
public:
    static constexpr int kStartAddress = 0x200;
    static constexpr std::array<uint8_t, 0x50> kSprites = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    Memory() {
        Reset();
    }

    uint8_t* getRawMemory() {
        return memory.data();
    }

    uint16_t* getRawStartAddressTable() {
        return startAddressTable.data();
    }

    uint8_t& operator[](int index) {
        return memory.at(index);
    }

    void Reset() {
        memory.fill(0);
        startAddressTable.fill(0);
        std::copy(begin(kSprites), end(kSprites), begin(memory));
    }

    std::array<std::unique_ptr<BasicBlock>, 0x1000> jumpTable = {0};
    std::array<uint16_t, 0x1000> startAddressTable = {0};
    std::array<uint8_t, 0x1000> memory = {0};
};

}

#endif  // CHIP8_MEMORY_H