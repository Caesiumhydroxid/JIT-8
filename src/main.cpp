#include <asmjit/asmjit.h>
#include <array>
#include <iostream>
#include <iterator>
#include "basicblock.h"
#include "parser.h"
#include "cpu.h"
#include "memory.h"
#include <SFML/Graphics.hpp>
#include <cstdlib>
using namespace asmjit;


int startJit(CPU &cpu) {
    JitRuntime rt;
    
    c8::Memory memory;
    srand(time(NULL));
    std::array<uint8_t,3*8>program {
        0x60,64,
        0x61,30,
        0xA0,0x00,
        0xD0,0x15,
        0x10,0x00
    };
    std::array<uint8_t,38> maze_rom ={
  0x60, 0x00, 0x61, 0x00, 0xa2, 0x22, 0xc2, 0x01, 0x32, 0x01, 0xa2, 0x1e,
  0xd0, 0x14, 0x70, 0x04, 0x30, 0x40, 0x12, 0x04, 0x60, 0x00, 0x71, 0x04,
  0x31, 0x20, 0x12, 0x04, 0x12, 0x1c, 0x80, 0x40, 0x20, 0x10, 0x20, 0x40,
  0x80, 0x10
};

    
    

    printf("Pointer %ld \n",&cpu.stack);
    
    std::copy(maze_rom.begin(),maze_rom.end(),memory.memory.begin()+0x200);

    uint16_t currentAddress = 0x200;
    for(int i=0;i<5000;i++){
        if(memory.jumpTable[currentAddress] == NULL)
        {
            auto res = Parser::parseBasicBlock(memory.memory, currentAddress);
            auto basicBlock = std::make_unique<BasicBlock>(std::move(res),cpu,memory,rt);
            memory.jumpTable[currentAddress] = std::move(basicBlock);
        }
        currentAddress = memory.jumpTable[currentAddress]->fn();
        cpu.printState();
        std::cout<<currentAddress<<std::endl;
        if(currentAddress == 0) break;
    }
    cpu.display.drawBytes();
    return 0;
}

struct threadData
{
    sf::RenderWindow* window;
    CPU* cpu;
};


int startSfml(struct threadData data);

int main()
{
    CPU cpu;
    sf::RenderWindow window(sf::VideoMode(640, 320), "My window");
    struct threadData data = {&window, &cpu};
    window.setActive(false);
    sf::Thread thread(&startSfml,data);
    thread.launch();
    startJit(cpu);
}

int startSfml(struct threadData data) {
    
    CPU* cpu = data.cpu;
    sf::RenderWindow* window = data.window;
    while (window->isOpen())
    {

        sf::Event event;
        while (window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window->close();
        }
        window->clear(sf::Color::Black);
        sf::RectangleShape shape(sf::Vector2f(10.f,10.f));
        shape.setFillColor(sf::Color(255,255,255));
        for(int y = 0; y < cpu->display.kHeight; y++)
        {
            for(int x = 0; x < cpu->display.kWidth/8;x++)
            {
                uint8_t prnt = cpu->display.data[y*cpu->display.kWidth/8+x];
                for(int i=0;i<8;i++)
                {
                    if(prnt&(1<<(7-i))){
                        shape.setPosition(10*(i+x*8),10*y);
                        window->draw(shape);
                    }
                }
            }
        }
        window->display();
    }

    return 0;
}

