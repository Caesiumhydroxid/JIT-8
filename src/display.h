// Copyright 2021 Betamark Pty Ltd. All rights reserved.
// Author: Shlomi Nissan (shlomi@betamark.com)

#ifndef CHIP8_DISPLAY_H
#define CHIP8_DISPLAY_H

#include <array>
#include <vector>
#include <cinttypes>
#include <algorithm>
#include <stdexcept>
#include <SFML/Graphics.hpp>


class Display {
public:
    static constexpr int kWidth = 64;
    static constexpr int kHeight = 32;
    static constexpr int kScale = 10;


    void clearScreen();
    void draw();
    void drawBytes();
    uint8_t data[kWidth/8 * kHeight] = {0};
};


#endif  // CHIP8_DISPLAY_H