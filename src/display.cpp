#include "display.h"
#include <iostream>

void Display::clearScreen()
{
}


void Display::draw()
{
    
}

void Display::drawBytes()
{
    for(int y=0;y<16;y++){
        
        for(int i=0;i<8;i++)
        {
            std::cout << std::hex << unsigned(data[y*kWidth/8+i]);
        }
        std::cout << std::endl;
    }    
}