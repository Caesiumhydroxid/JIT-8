#ifndef BASIC_BLOCK_H
#define BASIC_BLOCK_H

#include <cstdint>
#include <asmjit/asmjit.h>
#include <array>
#include <vector>
#include <memory>
#include <optional>
#include <bitset>
#include "asmjit/core/operand.h"
#include "types.h"
#include "memory.h"
#include "cpu.h"

typedef uint64_t (*BasicBlockFunction)(void);

class BasicBlock {

    public: 
        struct BasicBlockInformation {
            uint16_t startingAddress;
            uint16_t endAddress;
            std::bitset<CPU::AMOUNT_REGISTERS> usedRegisters;
            std::vector<c8::Opcode> instructions;
            bool writesToItself;
        };

        void generatePrologue(asmjit::x86::Compiler &cc, asmjit::x86::Gp cpubase,
                                    std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers);

        void generateEpilogue(asmjit::x86::Compiler &cc, asmjit::x86::Gp cpubase,
                                    const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers);

        void compile(CPU &cpu, c8::Memory &mem,asmjit::JitRuntime &rt);

    private:
        
        std::unique_ptr<BasicBlockInformation> info;
        asmjit::CodeHolder code;
        asmjit::StringLogger logger;

        std::optional<asmjit::Label> generateInstruction(c8::Opcode instr,
                                        const CPU &cpu, 
                                        uint16_t pc,
                                        asmjit::x86::Gp cpubase,
                                        c8::Memory &mem,
                                        asmjit::x86::Compiler &cc, 
                                        const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers);
        
        std::vector<asmjit::Label> jumpingLocations;

    public:
        BasicBlock( std::unique_ptr<BasicBlockInformation> information,
                    CPU &cpu, 
                    c8::Memory &mem,
                    asmjit::JitRuntime& rt);
        int getStartAddr();
        int getEndAddr();
        asmjit::Label getLabelAccodingToAddress(uint16_t address);
        BasicBlockFunction fn;
};

#endif
