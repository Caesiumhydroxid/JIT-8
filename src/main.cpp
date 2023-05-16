#include <asmjit/asmjit.h>
#include <array>
#include <iostream>
#include <iterator>
#include <vector>
#include <fstream>
#include "basicblock.h"
#include "parser.h"
#include "cpu.h"
#include "memory.h"
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include "corax.h"
#include "quirks.h"
#include <unistd.h>
using namespace asmjit;


int startJit(CPU &cpu, std::vector<uint8_t> &rom) {
    JitRuntime rt;
    c8::Memory memory;
    srand(time(NULL));

    std::array<uint8_t,10>program {
        0xD0,0x05,
        0xD0,0x05,
        0x10,0x00,
    };

    printf("Pointer %h \n",memory.memory.data());
    
    std::copy(rom.begin(),rom.end(),memory.memory.begin()+0x200);

    uint16_t currentAddress = 0x200;
    for(int i=0;i<500000000;i++){
        if(memory.jumpTable[currentAddress] == NULL)
        {
            auto res = Parser::parseBasicBlock(memory.memory, currentAddress);
            auto basicBlock = std::make_unique<BasicBlock>(std::move(res),cpu,memory,rt);
            memory.jumpTable[currentAddress] = std::move(basicBlock);
        }
        currentAddress = memory.jumpTable[currentAddress]->fn();
        //cpu.printState();
        //for(int i=0;i<10;i++){
        //    std::cout << std::dec << unsigned(memory[i+cpu.indexRegister]) <<std::endl;
        //}
        //std::cout<<"Ret: "<< std::hex<<currentAddress<<std::endl;
        //usleep(10000);
        if(currentAddress == 0) break;
    }
    //cpu.display.drawBytes();
    return 0;
}

struct threadData
{
    sf::RenderWindow* window;
    CPU* cpu;
};


int startSfml(struct threadData data);

int main(int argc, char *argv[])
{
    CPU cpu;
    std::ifstream file(argv[1], std::ios::binary);  // Replace "example.txt" with your file's path

    if (file) {
        // Find the file size
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // Create a vector to hold the file contents
        std::vector<uint8_t> buffer(fileSize);

        // Read the file byte-wise into the vector
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

        if (file) {
            // File reading succeeded
            std::cout << "File read successfully. Total bytes read: " << file.gcount() << std::endl;
            sf::RenderWindow window(sf::VideoMode(640, 320), "My window");
            struct threadData data = {&window, &cpu};
            window.setActive(false);
            sf::Thread thread(&startSfml,data);
            thread.launch();
            startJit(cpu,buffer);
        } else {
            // File reading failed
            std::cerr << "Error reading file." << std::endl;
        }

        file.close();
        
    }
}

int startSfml(struct threadData data) {
    
    CPU* cpu = data.cpu;
    sf::RenderWindow* window = data.window;
    window->setFramerateLimit(60);
    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            if (event.type == sf::Event::KeyPressed){
                cpu->setKeyState(event.key.code,true);
            }
            if (event.type == sf::Event::KeyReleased){
                cpu->setKeyState(event.key.code,false);
            }
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
        if(cpu->delayTimer >= 1)
        {
            cpu->delayTimer--;
        }
        window->display();
    }

    return 0;
}

