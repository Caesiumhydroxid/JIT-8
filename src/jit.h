#pragma once
#include "basicblock.h"
#include "constants.h"
#include "display.h"
#include "hardware.h"
#include "memory.h"
#include <asmjit/asmjit.h>
#include <iterator>


void invalidateAndRecompileIfWroteToOwnBlock(Hardware &hardware,
                                             asmjit::JitRuntime &rt,
                                             Memory &memory,
                                             uint16_t &currentAddress,
                                             uint64_t &returnAddress);

void compileNextBlockIfNeeded(Hardware &hardware, asmjit::JitRuntime &rt,
                              Memory &memory, uint16_t &currentAddress);

void markEndAddrTableToBasicBlock(Memory &memory,
                                std::unique_ptr<BasicBlock> &basicBlock);

void deleteAllDirtyBlocksRange(Memory &memory, uint16_t dirtyAddressStart,
                               uint16_t dirtyAddressEnd);

void deleteAllDirtyBlocks(Memory &memory, uint16_t dirtyAddress);