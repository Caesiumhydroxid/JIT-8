#include "basicblock.h"
#include <iostream>
#include <cstdio>
#include <stddef.h>
#include <chrono>
#include "asmjit/core/operand.h"
#include "parser.h"
#include "hardware.h"
#include "memory.h"
#include "instruction.h"
#include "constants.h"

using namespace asmjit;

BasicBlock::BasicBlock( std::unique_ptr<BasicBlockInformation> information,
                        Hardware &hardware, 
                        Memory &mem,
                        asmjit::JitRuntime &rt)
{
    this->info = std::move(information);
    compile(hardware,mem,rt);
}

void generateSleepCode(x86::Compiler &cc, uint64_t slowdown){
    //pushq $0    #0 nanoseconds
    //pushq $2    #2 seconds
    //leaq (%rbp),%rdi    #the time structure on the stack       
    //movq $35,%rax       #nanosleep syscall
    //movq $0,%rsi        #disable useless parameter           
    //syscall             
    cc.push(asmjit::x86::rax);
    cc.push(asmjit::x86::rdi);
    cc.push(asmjit::x86::rsi);
    cc.push(asmjit::x86::rcx);
    cc.push(asmjit::x86::r11);
    cc.push(slowdown);
    cc.push(0);
    cc.mov(asmjit::x86::rdi,asmjit::x86::rsp);
    cc.xor_(asmjit::x86::rsi,asmjit::x86::rsi);
    cc.mov(asmjit::x86::rax,35);
    cc.emit(asmjit::x86::Inst::kIdSyscall);
    cc.add(asmjit::x86::rsp,16);
    cc.pop(asmjit::x86::r11);
    cc.pop(asmjit::x86::rcx);
    cc.pop(asmjit::x86::rsi);
    cc.pop(asmjit::x86::rdi);
    cc.pop(asmjit::x86::rax);
}

void BasicBlock::compile(Hardware &hardware, Memory &mem,asmjit::JitRuntime &rt){
    
    uint16_t pc = this->info->startingAddress;
    code.init(rt.environment(),rt.cpuFeatures());
    #if LOGGING
    code.setLogger(&logger);
    #endif
    x86::Compiler cc(&code);
    cc.addFunc(FuncSignatureT<uint64_t>());

    auto HARDWARE_BASE = cc.newUIntPtr();
    cc.mov(HARDWARE_BASE,&hardware);
    std::array<x86::Gp,Hardware::AMOUNT_REGISTERS> registers;
    generatePrologue(cc,HARDWARE_BASE,registers);
    std::optional<Label> jumpLab = {};
    for(size_t i=0; i<info->instructions.size();i++){
        jumpingLocations.push_back(cc.newLabel());
    }
    int i=0;
    for(auto instr: info->instructions)
    {
        #if LOGGING
        std::cout << std::hex << unsigned(pc) << ": " << unsigned(instr.in) << std::endl;
        #endif 
        cc.bind(jumpingLocations[i]);
        auto tempLabel = generateInstruction(instr,hardware,pc,HARDWARE_BASE,mem,cc,registers);
        if(jumpLab.has_value()){
            cc.bind(jumpLab.value());
        }
        jumpLab = tempLabel;
        pc+=2;
        i++;
        if(hardware.slowdown != 0)
        {
            generateSleepCode(cc, hardware.slowdown);
        }
    }
    generateEpilogue(cc,HARDWARE_BASE,registers);
    cc.endFunc();
    cc.finalize();
    Error err = rt.add(&fn,&code);
    if (err) {
        #if LOGGING
        std::cout<< asmjit::DebugUtils::errorAsString(err) << std::endl;
        #endif
    }
    #if LOGGING
    std::cout<<logger.data() << std::endl;
    #endif
}

void BasicBlock::generatePrologue(asmjit::x86::Compiler &cc,  asmjit::x86::Gp hardwarebase,
                                    std::array<asmjit::x86::Gp,Hardware::AMOUNT_REGISTERS> &registers)
{
    
    for(int i=0; i<Hardware::AMOUNT_REGISTERS;i++)
    {
        if(info->usedRegisters.test(i))
        {
            #if LOGGING
            std::cout<<"Load reg "<< i << std::endl;
            #endif
            
            registers[i] = cc.newUInt8();
            auto memreg = x86::byte_ptr(hardwarebase, offsetof(Hardware, regs) + static_cast<uint8_t>(i) );
            cc.mov(registers[i],memreg);
        }
    }
}

void BasicBlock::generateEpilogue(asmjit::x86::Compiler &cc, asmjit::x86::Gp hardwarebase,
                                    const std::array<asmjit::x86::Gp,Hardware::AMOUNT_REGISTERS> &registers)
{
    for(int i=0; i<Hardware::AMOUNT_REGISTERS;i++)
    {
        if(info->usedRegisters.test(i))
        {
            auto memreg = x86::byte_ptr(hardwarebase, offsetof(Hardware, regs) + static_cast<uint8_t>(i) );
            cc.mov(memreg,registers[i]);
        }
    }
   
}

asmjit::Label BasicBlock::getLabelAccodingToAddress(uint16_t address)
{
    return jumpingLocations[(address-getStartAddr())/2];
}

int BasicBlock::getStartAddr()
{
    return (*this->info).startingAddress;
}
int BasicBlock::getEndAddr()
{
    return (*this->info).endAddress;
}

std::optional<asmjit::Label> BasicBlock::generateInstruction(Opcode instr,
                                    const Hardware &hardware, 
                                    uint16_t pc,
                                    asmjit::x86::Gp hardwarebase,
                                    Memory &mem, asmjit::x86::Compiler &cc,
                                    const std::array<asmjit::x86::Gp,Hardware::AMOUNT_REGISTERS> &registers)
{
    Instruction parsedInstr = Parser::parse(instr);
    
    switch (parsedInstr) {
        case Instruction::CLS:
            CLS(hardware, cc); return {};
        case Instruction::RET:
            RET(hardware, this, cc, hardwarebase, registers); return {};
        case Instruction::JMP:          JMP     (instr,this,cc,hardwarebase,registers); return {};
        case Instruction::CALL:         CALL    (instr,hardware,this,pc,cc,hardwarebase,registers); return {};
        case Instruction::SE_VX_KK:     return SE_VX_KK(instr, cc, registers);
        case Instruction::SNE_VX_KK:    return SNE_VX_KK(instr, cc, registers);
        case Instruction::SE_VX_VY:     return SE_VX_VY(instr, cc, registers);
        case Instruction::LD_VX_KK:
            LD_VX_KK(instr, cc, registers); return {};
        case Instruction::ADD_VX_KK:
            ADD_VX_KK(instr, cc, registers); return {};
        case Instruction::LD_VX_VY:
            LD_VX_VY(instr, cc, registers); return {};
        case Instruction::OR_VX_VY:
            OR_VX_VY(instr, cc, registers); return {};
        case Instruction::AND_VX_VY:
            AND_VX_VY(instr, cc, registers); return {};
        case Instruction::XOR_VX_VY:
            XOR_VX_VY(instr, cc, registers); return {};
        case Instruction::ADD_VX_VY:
            ADD_VX_VY(instr, cc, registers); return {};
        case Instruction::SUB_VX_VY:
            SUB_VX_VY(instr, cc, registers); return {};
        case Instruction::SHR_VX:
            SHR_VX(instr, cc, registers); return {};
        case Instruction::SUBN_VX_VY:
            SUBN_VX_VY(instr, cc, registers); return {};
        case Instruction::SHL_VX:
            SHL_VX(instr, cc, registers); return {};
        case Instruction::SNE_VX_VY:    return SNE_VX_VY(instr, cc, registers);
        case Instruction::LD_I:
            LD_I(instr, hardwarebase, cc); return {};
        case Instruction::JMP_V0:
            JMP_V0(instr, this, cc, hardwarebase, registers); return {};
        case Instruction::RND:
            RND(instr, hardwarebase, cc, registers); return{};
        case Instruction::DRW:          DRW(instr,hardware,hardwarebase,mem,cc,registers); return{};
        case Instruction::SKP:          return SKP(instr, hardware, cc, registers);
        case Instruction::SKNP:         return SKNP(instr, hardware, cc, registers);
        case Instruction::LD_VX_DT:
            LD_VX_DT(instr, hardwarebase, cc, registers); return {};
        case Instruction::LD_VX_K:
            LD_VX_K(instr, hardware, cc, registers); return {};
        case Instruction::LD_DT:
            LD_DT(instr, hardwarebase, cc, registers); return {};
        case Instruction::LD_ST:
            LD_ST(instr, hardwarebase, cc, registers); return {};
        case Instruction::ADD_I_VX:
            ADD_I_VX(instr, hardwarebase, cc, registers); return {};
        case Instruction::LD_F_VX:
            LD_F_VX(instr, hardwarebase, cc, registers); return {};
        case Instruction::LD_B_VX:
            LD_B_VX(instr, this, pc, hardwarebase, mem, cc, registers); return {};
        case Instruction::LD_I_VX:
            LD_I_VX(instr, this, pc, hardwarebase, mem, cc, registers); return {};
        case Instruction::LD_VX_I:
            LD_VX_I(instr, hardwarebase, mem, cc, registers); return {};
        case Instruction::UNKNOWN:      std::cout << "Missing Instruction" << std::endl; return {};
    }
    return {};
}
