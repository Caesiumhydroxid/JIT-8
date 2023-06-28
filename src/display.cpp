#include "display.h"
#include <iostream>


void Display::drawBytes()
{
    for(int y=0;y<16;y++){
        
        for(int i=0;i<8;i++)
        {
            std::cout << std::hex << unsigned(data[y*WIDTH/8+i]);
        }
        std::cout << std::endl;
    }    
}

void Display::drawToRenderWindow(sf::RenderWindow* const window)
{
    sf::RectangleShape shape(sf::Vector2f(10.f,10.f));
        shape.setFillColor(sf::Color(255,255,255));
        for(int y = 0; y < Display::HEIGHT; y++)
        {
            for(int x = 0; x < Display::WIDTH/8;x++)
            {
                uint8_t prnt = data[y*Display::WIDTH/8+x];
                for(int i=0;i<8;i++)
                {
                    if(prnt&(1<<(7-i))){
                        shape.setPosition(10*(i+x*8),10*y);
                        window->draw(shape);
                    }
                }
            }
        }
}