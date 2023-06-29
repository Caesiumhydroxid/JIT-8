#include "basicblock.h"
#include "constants.h"
#include "hardware.h"
#include "memory.h"
#include "parser.h"
#include "jit.h"
#include <SFML/Graphics.hpp>
#include <asmjit/asmjit.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <unistd.h>

bool iRegIncrementQuirk = true;
bool shiftQuirk = true;

typedef struct runtimeInformation_s {
  std::chrono::duration<double, std::micro> totalRuntime;
  std::chrono::duration<double, std::micro> compilerRuntime;
} runtimeInformation_t;


struct threadData {
  sf::RenderWindow *window;
  Hardware *hardware;
};

int startSfml(struct threadData data);
runtimeInformation_t startJit(Hardware &hardware, std::string path);

int main(int argc, char *argv[]) {
  int opt;
  uint32_t slowdown = 0;
  std::string filepath;
  
  // Shut GetOpt error messages down (return '?'):
  opterr = 0;

  // Retrieve the options:
  while ((opt = getopt(argc, argv, "t:is")) != -1) { // for each option...
    switch (opt) {
    case 't':
      slowdown = std::stoi(optarg);
      break;
    case 'i':
      iRegIncrementQuirk = false;
      break;
    case 's':
      shiftQuirk = false;
      break;
    case '?': // unknown option...
      std::cerr << "Usage: " << argv[0] << " <-t slowdown in ns> <-i reg i quirk disable>  <-s shift quirk disable> filepath_rom" << std::endl;
      exit(EXIT_FAILURE);
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

runtimeInformation_t startJit(Hardware &hardware, std::string path) {

  runtimeInformation_t timeInfo;
  timeInfo.totalRuntime = std::chrono::microseconds(0);
  timeInfo.compilerRuntime = std::chrono::microseconds(0);

  asmjit::JitRuntime rt;
  Memory memory;
  if (!memory.initializeFromFile(path)) {
    std::cerr << "Error loading Rom" << std::endl;
    exit(EXIT_FAILURE);
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
      exit(EXIT_FAILURE);
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
    if (currentAddress == 0){
      std::cout << "Finished" << std::endl;
      break;
    }
      
  }
  timeInfo.totalRuntime = std::chrono::high_resolution_clock::now() - startTime;
  return timeInfo;
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
  exit(EXIT_SUCCESS);
  return 0;
}
