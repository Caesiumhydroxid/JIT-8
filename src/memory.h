#pragma once

#include <array>
#include <cinttypes>
#include <memory>
#include "basicblock.h"

class Memory {
public:

    static constexpr int MEMORY_SIZE = 0x1000;
    static constexpr int START_ADDRESS = 0x200;
    static constexpr std::array<uint8_t, 0x50> SPRITES = {
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

    Memory();

    uint8_t* getRawMemory();

    uint16_t* getRawEndAddressTable();

    void Reset();

    bool initializeFromFile(std::string path);
    std::array<std::unique_ptr<BasicBlock>, MEMORY_SIZE> jumpTable = {0};
    std::array<uint16_t, MEMORY_SIZE> endAddressTable = {0};
    std::array<uint8_t, MEMORY_SIZE> memory = {0};
};