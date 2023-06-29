#include "jit.h"
#include "parser.h"

void deleteAllDirtyBlocks(Memory &memory, uint16_t dirtyAddress)
{
  for (size_t j = 0; j < memory.jumpTable.size(); j++)
  {
    if (memory.jumpTable[j] != nullptr)
    {
      if (memory.jumpTable[j]->getStartAddr() <= dirtyAddress &&
          memory.jumpTable[j]->getEndAddr() + 2 > dirtyAddress)
      {
#if LOGGING
        std::cout << "Block dirty at " << memory.jumpTable[j]->getStartAddr()
                  << " " << memory.jumpTable[j]->getEndAddr() << std::endl;
#endif

        for (int i = memory.jumpTable[j]->getStartAddr();
             i < memory.jumpTable[j]->getEndAddr() + 2; i++)
        {
          memory.endAddressTable[i] = 0;
        }

        memory.jumpTable[j].reset();
        memory.jumpTable[j] = nullptr;
      }
    }
  }
}

void deleteAllDirtyBlocksRange(Memory &memory, uint16_t dirtyAddressStart,
                               uint16_t dirtyAddressEnd)
{
  for (size_t j = 0; j < memory.jumpTable.size(); j++)
  {
    if (memory.jumpTable[j] != nullptr)
    {
      if (memory.jumpTable[j]->getStartAddr() <= dirtyAddressEnd &&
          dirtyAddressStart <= memory.jumpTable[j]->getEndAddr())
      {
        for (int i = memory.jumpTable[j]->getStartAddr();
             i < memory.jumpTable[j]->getEndAddr() + 2; i++)
        {
          memory.endAddressTable[i] = 0;
        }
        memory.jumpTable[j].reset();
        memory.jumpTable[j] = nullptr;
      }
    }
  }
}

void markEndAddrTableToBasicBlock(Memory &memory,
                                  std::unique_ptr<BasicBlock> &basicBlock)
{
  for (int i = basicBlock->getStartAddr(); i < basicBlock->getEndAddr() + 2;
       i++)
  {
    memory.endAddressTable[i] = basicBlock->getEndAddr() + 1;
  }
}

void compileNextBlockIfNeeded(Hardware &hardware, asmjit::JitRuntime &rt,
                              Memory &memory, uint16_t &currentAddress)
{
  if (memory.jumpTable[currentAddress] != nullptr)
  {
    if (memory.jumpTable[currentAddress]->getEndAddr() + 1 ==
        memory
            .endAddressTable[memory.jumpTable[currentAddress]->getEndAddr() +
                             1])
    { // Block not dirty
      return;
    }
    deleteAllDirtyBlocks(memory,
                         memory.jumpTable[currentAddress]->getEndAddr() +
                             1); // Find all dirty blocks and delete them
  }
  if (memory.jumpTable[currentAddress] == nullptr)
  {
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
                                             uint64_t &returnAddress)
{

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