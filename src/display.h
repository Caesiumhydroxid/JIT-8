#ifndef CHIP8_DISPLAY_H
#define CHIP8_DISPLAY_H

#include "SFML/Graphics/RenderWindow.hpp"
#include <array>
#include <vector>
#include <cinttypes>
#include <algorithm>
#include <stdexcept>
#include <SFML/Graphics.hpp>


class Display {
public:
    static constexpr int WIDTH = 64;
    static constexpr int HEIGHT = 32;
    static constexpr int SCALE = 10;

    void drawBytes();
    void drawToRenderWindow(sf::RenderWindow* const window);
    uint8_t data[WIDTH/8 * HEIGHT] = {0};
};


#endif  // CHIP8_DISPLAY_H