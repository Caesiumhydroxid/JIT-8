#include <asmjit/asmjit.h>
#include <array>
#include <bits/chrono.h>
#include <chrono>
#include <exception>
#include <iostream>
#include <iterator>
#include <ratio>
#include <vector>
#include <fstream>
#include "basicblock.h"
#include "parser.h"
#include "hardware.h"
#include "memory.h"
#include <SFML/Graphics.hpp>
#include <unistd.h>
#include "constants.h"
using namespace asmjit;

void deleteAllDirtyBlocks(c8::Memory &memory, uint16_t dirtyAddress) {
  for (size_t j = 0; j < memory.jumpTable.size(); j++) {
    if (memory.jumpTable[j] != nullptr) {
      if (memory.jumpTable[j]->getStartAddr() <= dirtyAddress &&
          memory.jumpTable[j]->getEndAddr()+2 > dirtyAddress) {

        std::cout << "Block dirty at " <<  memory.jumpTable[j]->getStartAddr() << " " << memory.jumpTable[j]->getEndAddr() << std::endl;
        
        for (int i = memory.jumpTable[j]->getStartAddr();
             i < memory.jumpTable[j]->getEndAddr()+2; i++) {
          memory.startAddressTable[i] = 0;
        }
        
        
        memory.jumpTable[j].reset();
        memory.jumpTable[j] = nullptr;
      }
    }
  }
}

void deleteAllDirtyBlocksRange(c8::Memory &memory, uint16_t dirtyAddressStart, uint16_t dirtyAddressEnd) {
  for (size_t j = 0; j < memory.jumpTable.size(); j++) {
    if (memory.jumpTable[j] != nullptr) {
      if (memory.jumpTable[j]->getStartAddr() <= dirtyAddressEnd &&
          dirtyAddressStart <= memory.jumpTable[j]->getEndAddr()) {
        for (int i = memory.jumpTable[j]->getStartAddr();
             i < memory.jumpTable[j]->getEndAddr()+2; i++) {
          memory.startAddressTable[i] = 0;
        }
        memory.jumpTable[j].reset();
        memory.jumpTable[j] = nullptr;
      }
    }
  }
}

void markStartTableToBasicBlock(c8::Memory &memory,
               std::unique_ptr<BasicBlock> &basicBlock) {
  for (int i = basicBlock->getStartAddr(); i < basicBlock->getEndAddr()+2; i++) {
    memory.startAddressTable[i] = basicBlock->getEndAddr()+1;
  }
}

void compileNextBlockIfNeeded(Hardware &cpu, asmjit::JitRuntime &rt, c8::Memory &memory, uint16_t &currentAddress) {
  if (memory.jumpTable[currentAddress] != nullptr) {
    if(memory.jumpTable[currentAddress]->getEndAddr()+1 == memory.startAddressTable[memory.jumpTable[currentAddress]->getEndAddr()+1]){ // Block not dirty
      return;
    }
    deleteAllDirtyBlocks(
        memory,  memory.jumpTable[currentAddress]->getEndAddr()+1 ); // Find all dirty blocks and delete them
  } 
  if (memory.jumpTable[currentAddress] == nullptr)  {
      auto res = Parser::parseBasicBlock(memory.memory, currentAddress);
      auto basicBlock =
          std::make_unique<BasicBlock>(std::move(res), cpu, memory, rt);
      markStartTableToBasicBlock(memory, basicBlock);
      memory.jumpTable[currentAddress] = std::move(basicBlock);
  }
}

void invalidateAndRecompileIfWroteToOwnBlock(Hardware &cpu, asmjit::JitRuntime &rt, c8::Memory &memory,
                                             uint16_t &currentAddress, uint64_t &returnAddress)
{

    uint16_t writeStartAddress = (returnAddress >> 16) & 0xFFF;
    // The block wrote to itself (and stoppped execution after this write)
    // Invalidate all dirty blocks
    uint16_t programCounterAtIssue = returnAddress & 0xFFF;

    std::cout << "It happened!! " << std::hex << programCounterAtIssue << " "
              << writeStartAddress << std::endl;

    deleteAllDirtyBlocksRange(memory, writeStartAddress,
                              writeStartAddress + 16);

    // We also compile a block which goes only up to the write so it should need
    // less recompiles
    auto res = Parser::parseBasicBlock(memory.memory, currentAddress,
                                       programCounterAtIssue + 2);
    auto basicBlock =
        std::make_unique<BasicBlock>(std::move(res), cpu, memory, rt);
    markStartTableToBasicBlock(memory, basicBlock);
    memory.jumpTable[currentAddress] = std::move(basicBlock);
    currentAddress = programCounterAtIssue + 2;
}

typedef struct runtimeInformation_s
{
  std::chrono::duration<double ,std::micro> totalRuntime;
  std::chrono::duration<double ,std::micro> compilerRuntime;
} runtimeInformation_t;

runtimeInformation_t startJit(Hardware &cpu, std::vector<uint8_t> &rom) {

  runtimeInformation_t timeInfo;
  timeInfo.totalRuntime = std::chrono::microseconds(0);
  timeInfo.compilerRuntime = std::chrono::microseconds(0);
  
  JitRuntime rt;
  c8::Memory memory;

  //Copy Rom to Memory
  std::copy(rom.begin(), rom.end(), memory.memory.begin() + 0x200);

  auto startTime = std::chrono::high_resolution_clock::now();
  uint16_t currentAddress = 0x200;
  while (true) {

    auto startCompile = std::chrono::high_resolution_clock::now();
    compileNextBlockIfNeeded(cpu, rt, memory, currentAddress);
    auto doneCompile = std::chrono::high_resolution_clock::now();
    timeInfo.compilerRuntime += doneCompile - startCompile;
    #if LOGGING
    std::cout << "Exec:" << std::hex << unsigned(currentAddress) << std::endl;
    #endif
    uint64_t returnAddress = memory.jumpTable[currentAddress]->fn();
    if(returnAddress&0x8000) {
      startCompile = std::chrono::high_resolution_clock::now();
        invalidateAndRecompileIfWroteToOwnBlock(cpu, rt, memory, currentAddress, returnAddress);
      doneCompile = std::chrono::high_resolution_clock::now();
      timeInfo.compilerRuntime += doneCompile - startCompile;
    }
    else {
      currentAddress = returnAddress & 0xFFF;
    }
    if (currentAddress == 0)
      break;
  }
  timeInfo.totalRuntime = std::chrono::high_resolution_clock::now() - startTime;
  return timeInfo;
} 

struct threadData
{
    sf::RenderWindow* window;
    Hardware* cpu;
};


int startSfml(struct threadData data);

int main(int argc, char *argv[])
{
    int opt;
    uint32_t slowdown = 0;
    std::string filepath;

    // Shut GetOpt error messages down (return '?'):
    opterr = 0;

    // Retrieve the options:
    while ( (opt = getopt(argc, argv, "t:")) != -1 ) {  // for each option...
        switch ( opt ) {
            case 't':
                slowdown = std::stoi(optarg);
                break;
            case '?':  // unknown option...
                std::cerr << "Usage chip8 <-t slowdown> filepath_rom";
                break;
        }
    }
    if(optind < argc)
    {
        filepath = argv[optind];
    }

    Hardware hardware;
    hardware.slowdown = slowdown;

    std::ifstream file(filepath, std::ios::binary);  // Replace "example.txt" with your file's path
    if (file) {
        // Find the file size
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // Create a vector to hold the file contents
        std::vector<uint8_t> buffer(fileSize);

        // Read the file byte-wise into the vector
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        if (buffer.data()) {
            // File reading succeeded
            std::cout << "File read successfully. Total bytes read: " << file.gcount() << std::endl;
            sf::RenderWindow window(sf::VideoMode(640, 320), "My window");
            struct threadData data = {&window, &hardware};
            window.setActive(false);
            sf::Thread thread(&startSfml,data);
            thread.launch();

            runtimeInformation_s timeInfo = startJit(hardware,buffer);
            
            
            std::cout << "Total Compiletime : " << timeInfo.compilerRuntime.count() << "μs" << std::endl;
            std::cout << "Total Runtime     : " << timeInfo.totalRuntime.count() << "μs" << std::endl;
            std::cout << "Compile percentage: " << timeInfo.compilerRuntime.count() / timeInfo.totalRuntime.count() << "%" << std::endl;
        } else {
            // File reading failed
            std::cerr << "Error reading file." << std::endl;
        }

        
        
    }
}

int startSfml(struct threadData data) {
    
    Hardware* cpu = data.cpu;
    sf::RenderWindow* window = data.window;
    window->setFramerateLimit(60);
    while (window->isOpen())
    {
        sf::Event event{};
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
        for(int y = 0; y < Display::kHeight; y++)
        {
            for(int x = 0; x < Display::kWidth/8;x++)
            {
                uint8_t prnt = cpu->display.data[y*Display::kWidth/8+x];
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
    exit(0);
    return 0;
}

