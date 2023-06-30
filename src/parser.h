#pragma once

#include "basicblock.h"
#include "types.h"
#include "memory.h"
#include <array>
#include <bitset>
#include <iostream>
#include <optional>

enum class Instruction {
  ADD_I_VX, ADD_VX_KK, ADD_VX_VY, AND_VX_VY,
  CALL,
  CLS, DRW,
  JMP, JMP_V0,
  LD_B_VX, LD_DT, LD_F_VX, LD_I, LD_I_VX, LD_ST, LD_VX_DT, LD_VX_I, LD_VX_K, LD_VX_KK, LD_VX_VY,
  OR_VX_VY,
  RET,
  RND,
  SE_VX_KK, SE_VX_VY,
  SHL_VX, SHR_VX,
  SKNP, SKP, SNE_VX_KK, SNE_VX_VY,
  SUBN_VX_VY, SUB_VX_VY, XOR_VX_VY,
  UNKNOWN
};

class Parser {
public:

  template <std::size_t N>
  static std::unique_ptr<BasicBlock::BasicBlockInformation>
  parseBasicBlock(typename std::array<uint8_t, N> code, int pos,
                  size_t maxPos = Memory::MEMORY_SIZE) {
    auto blockInformation =
        std::make_unique<BasicBlock::BasicBlockInformation>();
    uint16_t amountInstructions = 0;
    blockInformation->startingAddress = pos;
    blockInformation->usedRegisters = std::bitset<Hardware::AMOUNT_REGISTERS>();
    blockInformation->endAddress = pos;
    auto prevInstr = std::optional<Instruction>{};

    if (pos > 1) {
      uint16_t assembledInstruction = code[pos - 2] << 8 | code[pos - 1];
      prevInstr = parse(assembledInstruction);
    }

    for (size_t i = pos; i < code.size() && i < maxPos; i++) {
      uint16_t assembledInstruction = code[i] << 8;
      i++;
      assembledInstruction |= code[i];
      auto parsedInstr = parse(assembledInstruction);
      if(parsedInstr == Instruction::UNKNOWN){
        //Unknown Instruction Ends Basic Block
        break;
      }
      blockInformation->usedRegisters |=
          parseUsedRegisters(assembledInstruction);
      blockInformation->instructions.emplace_back(assembledInstruction);
      amountInstructions++;
      if (isJumpInstruction(parsedInstr, prevInstr)) {
        break;
      }
      
      prevInstr = parsedInstr;
    }
    blockInformation->endAddress = pos + (amountInstructions)*2;
    return blockInformation;
  }

  static Instruction parse(Opcode instr);
  static std::bitset<Hardware::AMOUNT_REGISTERS>
  parseUsedRegisters(Opcode instr);

private:
  static bool isJumpInstruction(Instruction instr,
                                std::optional<Instruction> before);
  static bool isWriteInstruction(Instruction instr);
};