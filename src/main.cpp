#include "basicblock.h"
#include "constants.h"
#include "display.h"
#include "hardware.h"
#include "memory.h"
#include "parser.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <asmjit/asmjit.h>
#include <bits/chrono.h>
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ratio>
#include <unistd.h>
#include <vector>
using namespace asmjit;

void deleteAllDirtyBlocks(Memory &memory, uint16_t dirtyAddress) {
  for (size_t j = 0; j < memory.jumpTable.size(); j++) {
    if (memory.jumpTable[j] != nullptr) {
      if (memory.jumpTable[j]->getStartAddr() <= dirtyAddress &&
          memory.jumpTable[j]->getEndAddr() + 2 > dirtyAddress) {

#if LOGGING
        std::cout << "Block dirty at " << memory.jumpTable[j]->getStartAddr()
                  << " " << memory.jumpTable[j]->getEndAddr() << std::endl;
#endif

        for (int i = memory.jumpTable[j]->getStartAddr();
             i < memory.jumpTable[j]->getEndAddr() + 2; i++) {
          memory.endAddressTable[i] = 0;
        }

        memory.jumpTable[j].reset();
        memory.jumpTable[j] = nullptr;
      }
    }
  }
}

void deleteAllDirtyBlocksRange(Memory &memory, uint16_t dirtyAddressStart,
                               uint16_t dirtyAddressEnd) {
  for (size_t j = 0; j < memory.jumpTable.size(); j++) {
    if (memory.jumpTable[j] != nullptr) {
      if (memory.jumpTable[j]->getStartAddr() <= dirtyAddressEnd &&
          dirtyAddressStart <= memory.jumpTable[j]->getEndAddr()) {
        for (int i = memory.jumpTable[j]->getStartAddr();
             i < memory.jumpTable[j]->getEndAddr() + 2; i++) {
          memory.endAddressTable[i] = 0;
        }
        memory.jumpTable[j].reset();
        memory.jumpTable[j] = nullptr;
      }
    }
  }
}

void markEndAddrTableToBasicBlock(Memory &memory,
                                std::unique_ptr<BasicBlock> &basicBlock) {
  for (int i = basicBlock->getStartAddr(); i < basicBlock->getEndAddr() + 2;
       i++) {
    memory.endAddressTable[i] = basicBlock->getEndAddr() + 1;
  }
}

void compileNextBlockIfNeeded(Hardware &hardware, asmjit::JitRuntime &rt,
                              Memory &memory, uint16_t &currentAddress) {
  if (memory.jumpTable[currentAddress] != nullptr) {
    if (memory.jumpTable[currentAddress]->getEndAddr() + 1 ==
        memory
            .endAddressTable[memory.jumpTable[currentAddress]->getEndAddr() +
                               1]) { // Block not dirty
      return;
    }
    deleteAllDirtyBlocks(memory,
                         memory.jumpTable[currentAddress]->getEndAddr() +
                             1); // Find all dirty blocks and delete them
  }
  if (memory.jumpTable[currentAddress] == nullptr) {
    auto res = Parser::parseBasicBlock(memory.memory, currentAddress);

    auto basicBlock =
        std::make_unique<BasicBlock>(std::move(res), hardware, memory, rt);

    markEndAddrTableToBasicBlock(memory, basicBlock);
    memory.jumpTable[currentAddress] = std::move(basicBlock);
  }
}

void invalidateAndRecompileIfWroteToOwnBlock(Hardware &hardware,
                                             asmjit::JitRuntime &rt,
                                             Memory &memory,
                                             uint16_t &currentAddress,
                                             uint64_t &returnAddress) {

  uint16_t writeStartAddress = (returnAddress >> 16) & 0xFFF;
  // The block wrote to itself (and stoppped execution after this write)
  // Invalidate all dirty blocks
  uint16_t programCounterAtIssue = returnAddress & 0xFFF;

#if LOGGING
  std::cout << "It happened!! " << std::hex << programCounterAtIssue << " "
            << writeStartAddress << std::endl;
#endif

  deleteAllDirtyBlocksRange(memory, writeStartAddress, writeStartAddress + 16);

  // We also compile a block which goes only up to the write so it should need
  // less recompiles
  auto res = Parser::parseBasicBlock(memory.memory, currentAddress,
                                     programCounterAtIssue + 2);
  auto basicBlock =
      std::make_unique<BasicBlock>(std::move(res), hardware, memory, rt);
  markEndAddrTableToBasicBlock(memory, basicBlock);
  memory.jumpTable[currentAddress] = std::move(basicBlock);
  currentAddress = programCounterAtIssue + 2;
}

typedef struct runtimeInformation_s {
  std::chrono::duration<double, std::micro> totalRuntime;
  std::chrono::duration<double, std::micro> compilerRuntime;
} runtimeInformation_t;

runtimeInformation_t startJit(Hardware &hardware, std::string path) {

  runtimeInformation_t timeInfo;
  timeInfo.totalRuntime = std::chrono::microseconds(0);
  timeInfo.compilerRuntime = std::chrono::microseconds(0);

  JitRuntime rt;
  Memory memory;
  if (!memory.initializeFromFile(path)) {
    std::cerr << "Error Loading Rom" << std::endl;
    exit(1);
  }

  auto startTime = std::chrono::high_resolution_clock::now();
  uint16_t currentAddress = Memory::START_ADDRESS;

  while (true) {

    auto startCompile = std::chrono::high_resolution_clock::now();
    compileNextBlockIfNeeded(hardware, rt, memory, currentAddress);
    auto doneCompile = std::chrono::high_resolution_clock::now();
    timeInfo.compilerRuntime += doneCompile - startCompile;

#if LOGGING
    std::cout << "Exec:" << std::hex << unsigned(currentAddress) << std::endl;
#endif

    uint64_t returnAddress = memory.jumpTable[currentAddress]->fn();
    if (returnAddress == (((uint64_t)0) - 1)) {
      std::cerr << "Buffer Overflow within the Rom" << std::endl;
      exit(1);
    }

    if (returnAddress & 0x8000) {
      startCompile = std::chrono::high_resolution_clock::now();
      invalidateAndRecompileIfWroteToOwnBlock(hardware, rt, memory,
                                              currentAddress, returnAddress);
      doneCompile = std::chrono::high_resolution_clock::now();
      timeInfo.compilerRuntime += doneCompile - startCompile;
    } else {
      currentAddress = returnAddress & 0xFFF;
    }
    if (currentAddress == 0)
      break;
  }
  timeInfo.totalRuntime = std::chrono::high_resolution_clock::now() - startTime;
  return timeInfo;
}

struct threadData {
  sf::RenderWindow *window;
  Hardware *hardware;
};

int startSfml(struct threadData data);

int main(int argc, char *argv[]) {
  int opt;
  uint32_t slowdown = 0;
  std::string filepath;

  // Shut GetOpt error messages down (return '?'):
  opterr = 0;

  // Retrieve the options:
  while ((opt = getopt(argc, argv, "t:")) != -1) { // for each option...
    switch (opt) {
    case 't':
      slowdown = std::stoi(optarg);
      break;
    case '?': // unknown option...
      std::cerr << "Usage chip8 <-t slowdown> filepath_rom";
      break;
    }
  }
  if (optind < argc) {
    filepath = argv[optind];
  }

  Hardware hardware;
  hardware.slowdown = slowdown;

  sf::RenderWindow window(sf::VideoMode(640, 320), "My window");
  struct threadData data = {&window, &hardware};
  window.setActive(false);
  sf::Thread thread(&startSfml, data);
  thread.launch();
  runtimeInformation_s timeInfo = startJit(hardware, filepath);

  std::cout << "Total Compiletime : " << timeInfo.compilerRuntime.count()
            << "μs" << std::endl;
  std::cout << "Total Runtime     : " << timeInfo.totalRuntime.count() << "μs"
            << std::endl;
  std::cout << "Compile percentage: "
            << timeInfo.compilerRuntime.count() / timeInfo.totalRuntime.count()
            << "%" << std::endl;
}

int startSfml(struct threadData data) {

  Hardware *hardware = data.hardware;
  sf::RenderWindow *window = data.window;
  window->setFramerateLimit(60);
  while (window->isOpen()) {
    sf::Event event{};
    while (window->pollEvent(event)) {
      if (event.type == sf::Event::KeyPressed) {
        hardware->setKeyState(event.key.code, true);
      }
      if (event.type == sf::Event::KeyReleased) {
        hardware->setKeyState(event.key.code, false);
      }
      if (event.type == sf::Event::Closed)
        window->close();
    }
    window->clear(sf::Color::Black);
    data.hardware->display.drawToRenderWindow(window);

    if (hardware->delayTimer >= 1) {
      hardware->delayTimer--;
    }
    window->display();
  }
  exit(0);
  return 0;
}
