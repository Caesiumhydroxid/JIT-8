#pragma once
#include "basicblock.h"
#include "globals.h"
#include "display.h"
#include "hardware.h"
#include "memory.h"
#include <asmjit/asmjit.h>


void invalidateAndRecompileIfWroteToOwnBlock(Hardware &hardware,
                                             asmjit::JitRuntime &rt,
                                             Memory &memory,
                                             uint16_t &currentAddress,
                                             uint64_t &returnAddress);

bool compileNextBlockIfNeeded(Hardware &hardware, asmjit::JitRuntime &rt,
                              Memory &memory, uint16_t &currentAddress);

void markEndAddrTableToBasicBlock(Memory &memory,
                                std::unique_ptr<BasicBlock> &basicBlock);

void deleteAllDirtyBlocksRange(Memory &memory, uint16_t dirtyAddressStart,
                               uint16_t dirtyAddressEnd);

void deleteAllDirtyBlocks(Memory &memory, uint16_t dirtyAddress);