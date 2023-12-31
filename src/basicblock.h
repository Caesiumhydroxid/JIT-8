#pragma once

#include <cstdint>
#include <asmjit/asmjit.h>
#include <array>
#include <vector>
#include <memory>
#include <optional>
#include <bitset>
#include "asmjit/core/operand.h"
#include "types.h"

#include "hardware.h"

class Memory;
typedef uint64_t (*BasicBlockFunction)(void);

class BasicBlock {

    public: 
        struct BasicBlockInformation {
            uint16_t startingAddress;
            uint16_t endAddress;
            std::bitset<Hardware::AMOUNT_REGISTERS> usedRegisters;
            std::vector<Opcode> instructions;
            bool writesToItself;
        };

        void generatePrologue(asmjit::x86::Compiler &cc, asmjit::x86::Gp hardwarebase,
                                    std::array<asmjit::x86::Gp,Hardware::AMOUNT_REGISTERS> &registers);

        void generateEpilogue(asmjit::x86::Compiler &cc, asmjit::x86::Gp hardwarebase,
                                    const std::array<asmjit::x86::Gp,Hardware::AMOUNT_REGISTERS> &registers);

        void compile(Hardware &hardware, Memory &mem,asmjit::JitRuntime &rt);

    private:
        
        std::unique_ptr<BasicBlockInformation> info;
        asmjit::CodeHolder code;
        asmjit::StringLogger logger;

        std::optional<asmjit::Label> generateInstruction(Opcode instr,
                                        const Hardware &hardware, 
                                        uint16_t pc,
                                        asmjit::x86::Gp hardwarebase,
                                        Memory &mem,
                                        asmjit::x86::Compiler &cc, 
                                        const std::array<asmjit::x86::Gp,Hardware::AMOUNT_REGISTERS> &registers);
        
        std::vector<asmjit::Label> jumpingLocations;

    public:
        BasicBlock( std::unique_ptr<BasicBlockInformation> information,
                    Hardware &hardware, 
                    Memory &mem,
                    asmjit::JitRuntime& rt);
        int getStartAddr();
        int getEndAddr();

        asmjit::Label getLabelAccodingToAddress(uint16_t address);
        /**
         * @brief This is the compiled function for this block
         * @return A uint64_t with following structure: The first 12bits indicate the address of the next block to be called
         * The bitmask 0x8000 (ie. the bit 15) indicates that a write to the own block occured)
         * The bitmask 0x1FFF0000 (bits 28-16) indicate the (first) dirty address (writes can be up to 16 bytes)
         */
        BasicBlockFunction fn;
};
