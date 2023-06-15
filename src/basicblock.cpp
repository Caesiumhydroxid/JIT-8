#include "basicblock.h"
#include <iostream>
#include <cstdio>
#include <stddef.h>
#include "parser.h"
#include "cpu.h"
#include "memory.h"
#include "instruction.h"

using namespace asmjit;

BasicBlock::BasicBlock( std::unique_ptr<BasicBlockInformation> information,
                        CPU &cpu, 
                        c8::Memory &mem,
                        asmjit::JitRuntime &rt)
{
    uint16_t pc = information->startingAddress;
    code.init(rt.environment(),rt.cpuFeatures());
    code.setLogger(&logger);
    x86::Compiler cc(&code);
    this->info = std::move(information);
    auto func = cc.addFunc(FuncSignatureT<uint16_t>());
    auto CPU_BASE = cc.newUIntPtr();
    cc.mov(CPU_BASE,&cpu);
    std::array<x86::Gp,CPU::AMOUNT_REGISTERS> registers;
    generatePrologue(cc,CPU_BASE,registers);
    std::optional<Label> jumpLab = {};
    for(auto instr: info->instructions)
    {
        std::cout << std::hex << unsigned(instr.in) << std::endl;
        auto tempLabel = generateInstruction(instr,cpu,pc,CPU_BASE,mem,cc,registers);
        if(jumpLab.has_value()){
            cc.bind(jumpLab.value());
        }
        jumpLab = tempLabel;
        pc+=2;
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
        cc.push(1000000);
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
    generateEpilogue(cc,CPU_BASE,registers);
    cc.endFunc();
    std::cout<< "Try" << std::endl;
    cc.finalize();

    Error err = rt.add(&fn,&code);
    if (err) {
        std::cout<< asmjit::DebugUtils::errorAsString(err) << std::endl;
    }
    std::cout<<logger.data() << std::endl;
}

void BasicBlock::generatePrologue(asmjit::x86::Compiler &cc,  asmjit::x86::Gp cpubase,
                                    std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    
    for(int i=0; i<CPU::AMOUNT_REGISTERS;i++)
    {
        if(info->usedRegisters.test(i))
        {
            std::cout<<"Load reg "<< i << std::endl;
            registers[i] = cc.newUInt8();
            auto memreg = x86::byte_ptr(cpubase, offsetof(CPU, regs) + static_cast<uint8_t>(i) );
            cc.mov(registers[i],memreg);
        }
    }
}

void BasicBlock::generateEpilogue(asmjit::x86::Compiler &cc, asmjit::x86::Gp cpubase,
                                    const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    for(int i=0; i<CPU::AMOUNT_REGISTERS;i++)
    {
        if(info->usedRegisters.test(i))
        {
            auto memreg = x86::byte_ptr(cpubase, offsetof(CPU, regs) + static_cast<uint8_t>(i) );
            cc.mov(memreg,registers[i]);
        }
    }
   
}

std::optional<asmjit::Label> BasicBlock::generateInstruction(c8::Opcode instr,
                                    const CPU &cpu, 
                                    uint16_t pc,
                                    asmjit::x86::Gp cpubase,
                                    c8::Memory &mem, asmjit::x86::Compiler &cc,
                                    const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    Instruction parsedInstr = Parser::parse(instr);
    
    using namespace c8::instruction;
    switch (parsedInstr) {
        case Instruction::CLS:          CLS     (instr,cpu,this,cc,cpubase,registers); return {};
        case Instruction::RET:          RET     (instr,cpu,this,cc,cpubase,registers); return {};
        case Instruction::JMP:          JMP     (instr,this,cc,cpubase,registers); return {};
        case Instruction::CALL:         CALL    (instr,cpu,this,pc,cc,cpubase,registers); return {};
        case Instruction::SE_VX_KK:     return SE_VX_KK  (instr,cpu,cc,registers);
        case Instruction::SNE_VX_KK:    return SNE_VX_KK (instr,cpu,cc,registers);
        case Instruction::SE_VX_VY:     return SE_VX_VY  (instr,cpu,cc,registers);
        case Instruction::LD_VX_KK:     LD_VX_KK  (instr,cpu,cc,registers); return {};
        case Instruction::ADD_VX_KK:    ADD_VX_KK (instr,cpu,cc,registers); return {};
        case Instruction::LD_VX_VY:     LD_VX_VY  (instr,cpu,cc,registers); return {};
        case Instruction::OR_VX_VY:     OR_VX_VY  (instr,cpu,cc,registers); return {};
        case Instruction::AND_VX_VY:    AND_VX_VY (instr,cpu,cc,registers); return {};
        case Instruction::XOR_VX_VY:    XOR_VX_VY (instr,cpu,cc,registers); return {};
        case Instruction::ADD_VX_VY:    ADD_VX_VY (instr,cpu,cc,registers); return {};
        case Instruction::SUB_VX_VY:    SUB_VX_VY (instr,cpu,cc,registers); return {};
        case Instruction::SHR_VX:       SHR_VX    (instr,cpu,cc,registers); return {};
        case Instruction::SUBN_VX_VY:   SUBN_VX_VY(instr,cpu,cc,registers); return {};
        case Instruction::SHL_VX:       SHL_VX    (instr,cpu,cc,registers); return {};
        case Instruction::SNE_VX_VY:    return SNE_VX_VY (instr,cpu,cc,registers);
        case Instruction::LD_I:         LD_I      (instr,cpu,cpubase,cc,registers); return {};
        case Instruction::JMP_V0:       JMP_V0    (instr, cpu, this ,cc,cpubase, registers); return {};
        case Instruction::RND:          RND       (instr,cpu,cpubase,cc,registers); return{};
        case Instruction::DRW:          DRW(instr,cpu,cpubase,mem,cc,registers); return{};
        case Instruction::SKP:          return SKP(instr,cpu,cpubase,cc,registers);
        case Instruction::SKNP:         return SKNP(instr,cpu,cpubase,cc,registers);
        case Instruction::LD_VX_DT:     LD_VX_DT  (instr,cpu,cpubase,cc,registers); return {};
        case Instruction::LD_VX_K:      LD_VX_K   (instr,cpu,cpubase,cc,registers); return {};
        case Instruction::LD_DT:        LD_DT     (instr,cpu,cpubase,cc,registers); return {};
        case Instruction::LD_ST:        LD_ST     (instr,cpu,cpubase,cc,registers); return {};
        case Instruction::ADD_I_VX:     ADD_I_VX  (instr,cpu,cpubase,cc,registers); return {};
        case Instruction::LD_F_VX:      LD_F_VX   (instr,cpu,cpubase,mem,cc,registers); return {};
        case Instruction::LD_B_VX:      LD_B_VX   (instr,cpu,cpubase,mem,cc,registers); return {};
        case Instruction::LD_I_VX:      LD_I_VX   (instr,cpu,cpubase,mem,cc,registers); return {};
        case Instruction::LD_VX_I:      LD_VX_I   (instr,cpu,cpubase,mem,cc,registers); return {};
    }
    std::cout << "Missing Instruction" << std::endl;
    return {};
}