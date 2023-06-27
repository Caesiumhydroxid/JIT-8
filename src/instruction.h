
#ifndef CHIP8_INSTRUCTIONS_H
#define CHIP8_INSTRUCTIONS_H

#include <asmjit/asmjit.h>
#include "basicblock.h"
#include "cpu.h"
#include "types.h"
#include "memory.h"
#include <optional>
#include<cstdlib>

namespace c8::instruction {

// 00E0 - Clear the display.
    void
    CLS(const CPU &cpu, asmjit::x86::Compiler &cc)
{
    auto simDisp = cc.newUIntPtr();
    cc.mov(simDisp,reinterpret_cast<uint64_t>(cpu.display.data));
    for(size_t i=0;i<sizeof(cpu.display.data)/(64/8);i++)
    {
        cc.mov(asmjit::x86::ptr_64(simDisp,i*(64/8)),0);
    }

}

// 00EE - Return from a subroutine.
    void RET(const CPU &cpu, BasicBlock *basicBlock, asmjit::x86::Compiler &cc, asmjit::x86::Gp cpubase,
             const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers)
{
    basicBlock->generateEpilogue(cc,cpubase,registers);
    auto ptr = cc.newUIntPtr();
    auto stackPointer = asmjit::x86::ptr_16(cpubase,offsetof(CPU, sp));
    auto stackMem = cc.newUIntPtr();
    cc.xor_(stackMem,stackMem);
    cc.dec(stackPointer);
    cc.mov(stackMem.r16(),stackPointer);
    cc.shl(stackMem,1);
    cc.mov(ptr,reinterpret_cast<uint64_t>(&cpu.stack[0]));
    cc.add(ptr,stackMem);
    auto val = cc.newUInt64();
    cc.xor_(val,val);
    cc.mov(val.r16(), asmjit::x86::ptr_16(ptr));
    cc.ret(val);
}

// 1nnn - Jump to location nnn.
void JMP(Opcode instr, BasicBlock* basicBlock ,asmjit::x86::Compiler &cc,asmjit::x86::Gp cpubase, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    if(basicBlock->getStartAddr() <= instr.address() && instr.address() <= basicBlock->getEndAddr())
    {
        std::cout << "StartAddr:" << basicBlock->getStartAddr() << "EndAddr:" << basicBlock->getEndAddr() << "Jump: " << instr.address() << std::endl; 
        cc.jmp(basicBlock->getLabelAccodingToAddress(instr.address()));
    }
    else{
        basicBlock->generateEpilogue(cc,cpubase,registers);
        auto val = cc.newUInt64();
        cc.xor_(val,val);
        cc.mov(val.r16(), instr.address());
        cc.ret(val);
    }
}

// 2nnn - Call subroutine at nnn.
void CALL(Opcode instr, const CPU &cpu, BasicBlock* basicBlock, uint16_t pc ,asmjit::x86::Compiler &cc,asmjit::x86::Gp cpubase, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    auto ptr = cc.newUIntPtr();
    auto stackPointer = asmjit::x86::ptr_16(cpubase,offsetof(CPU, sp));
    auto stackMem = cc.newUIntPtr();
    cc.xor_(stackMem,stackMem);
    cc.mov(stackMem.r16(),stackPointer);
    cc.shl(stackMem,1);
    cc.mov(ptr,reinterpret_cast<uint64_t>(&cpu.stack[0]));
    cc.add(ptr,stackMem);
    cc.mov(asmjit::x86::ptr_16(ptr), pc+2);
    cc.inc(stackPointer);

    if(basicBlock->getStartAddr() <= instr.address() && instr.address() <= basicBlock->getEndAddr())
    {
        std::cout << "StartAddr:" << basicBlock->getStartAddr() << "EndAddr:" << basicBlock->getEndAddr() << "Jump: " << instr.address() << std::endl; 
        cc.jmp(basicBlock->getLabelAccodingToAddress(instr.address()));
    }
    else{
        basicBlock->generateEpilogue(cc,cpubase,registers);
        auto val = cc.newUInt64();
        cc.xor_(val,val);
        cc.mov(val.r16(), instr.address());
        cc.ret(val);
    }
}
// 3xkk - Skip next instruction if Vx = kk.
    std::optional<asmjit::Label> SE_VX_KK(Opcode instr, asmjit::x86::Compiler &cc,
                                          const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers)
{
    asmjit::Label ljump = cc.newLabel();
    cc.cmp(registers[instr.x()],instr.byte());
    cc.je(ljump);
    return ljump;
}

// 4xkk - Skip next instruction if Vx != kk.
    std::optional<asmjit::Label> SNE_VX_KK(Opcode instr, asmjit::x86::Compiler &cc,
                                           const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers)
{
    asmjit::Label ljump = cc.newLabel();
    cc.cmp(registers[instr.x()],instr.byte());
    cc.jne(ljump);
    return ljump;
}

// 5xy0 - Skip next instruction if Vx = Vy.
    std::optional<asmjit::Label> SE_VX_VY(Opcode instr, asmjit::x86::Compiler &cc,
                                          const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers)
{
    asmjit::Label ljump = cc.newLabel();
    cc.cmp(registers[instr.x()],registers[instr.y()]);
    cc.je(ljump);
    return ljump;
}
// 6xkk - The interpreter puts the value kk into register Vx.
    void LD_VX_KK(Opcode instr, asmjit::x86::Compiler &cc,
                  const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    cc.mov(registers[instr.x()],instr.byte());
}

// 7xkk - Adds the value kk to the value of register Vx.
    void ADD_VX_KK(Opcode instr, asmjit::x86::Compiler &cc,
                   const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    cc.add(registers[instr.x()],instr.byte());
}

// 8xy0 - Stores the value of register Vy in register Vx.
    void LD_VX_VY(Opcode instr, asmjit::x86::Compiler &cc,
                  const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    cc.mov(registers[instr.x()],registers[instr.y()]);
}

// 8xy1 - Performs a bitwise OR on the values of Vx and Vy.
    void OR_VX_VY(Opcode instr, asmjit::x86::Compiler &cc,
                  const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    cc.or_(registers[instr.x()],registers[instr.y()]);
}

// 8xy2 - Performs a bitwise AND on the values of Vx and Vy.
    void AND_VX_VY(Opcode instr, asmjit::x86::Compiler &cc,
                   const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    cc.and_(registers[instr.x()],registers[instr.y()]);
}

// 8xy3 - Performs a bitwise exclusive OR on the values of Vx and Vy.
    void XOR_VX_VY(Opcode instr, asmjit::x86::Compiler &cc,
                   const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    cc.xor_(registers[instr.x()],registers[instr.y()]);
}

// 8xy4 - Set Vx = Vx + Vy, set VF = carry.
    void ADD_VX_VY(Opcode instr, asmjit::x86::Compiler &cc,
                   const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    cc.add(registers[instr.x()],registers[instr.y()]);
    cc.setc(registers[CPU::REG_F]);
}

// 8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.
    void SUB_VX_VY(Opcode instr, asmjit::x86::Compiler &cc,
                   const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    cc.sub(registers[instr.x()],registers[instr.y()]);
    cc.setnc(registers[CPU::REG_F]);
}

// 8xy6 - Set Vx = Vx SHR 1.
    void SHR_VX(Opcode instr, asmjit::x86::Compiler &cc,
                const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    //quirks
    cc.mov(registers[instr.x()],registers[instr.y()]);
    cc.shr(registers[instr.x()],1);
    cc.setc(registers[CPU::REG_F]);
}

// 8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow.
    void SUBN_VX_VY(Opcode instr, asmjit::x86::Compiler &cc,
                    const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto tmp = cc.newUInt8();
    cc.mov(tmp,registers[instr.y()]);
    cc.sub(tmp,registers[instr.x()]);
    cc.mov(registers[instr.x()],tmp);
    cc.setnc(registers[CPU::REG_F]);
}

// 8xyE - Set Vx = Vx SHL 1.
    void SHL_VX(Opcode instr, asmjit::x86::Compiler &cc,
                const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    //quirks
    cc.mov(registers[instr.x()],registers[instr.y()]);
    cc.shl(registers[instr.x()],1);
    cc.setc(registers[15]);
}

// 9xy0 - Skip next instruction if Vx != Vy.
    std::optional<asmjit::Label> SNE_VX_VY(Opcode instr, asmjit::x86::Compiler &cc,
                                           const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers)
{
    asmjit::Label ljump = cc.newLabel();
    cc.cmp(registers[instr.x()],registers[instr.y()]);
    cc.jne(ljump);
    return ljump;
}

// Annn - Register I is set to nnn.
    void LD_I(Opcode instr, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc) {
    auto memreg = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.mov(memreg,instr.address());
}

// Bnnn - Program counter is set to nnn plus the value of V0.
    void JMP_V0(Opcode instr, BasicBlock *basicBlock, asmjit::x86::Compiler &cc, asmjit::x86::Gp cpubase,
                const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    basicBlock->generateEpilogue(cc,cpubase,registers);
    auto retVal = cc.newUInt64();
    cc.xor_(retVal,retVal);
    cc.mov(retVal.r8(),registers[0]);
    cc.add(retVal,instr.address());
    cc.ret(retVal);
}

// Cxkk - Set Vx = random byte AND kk
    void RND(Opcode instr, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc,
             const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto lfsrReg = asmjit::x86::ptr_32(cpubase,offsetof(CPU, rng));
    auto b = cc.newUInt32();
    cc.mov(b,lfsrReg);
    cc.and_(b,1);
    cc.neg(b);
    cc.and_(b,0xc3308398);
    cc.shr(lfsrReg,1);
    cc.xor_(lfsrReg,b);
    cc.mov(registers[instr.x()].r8(),lfsrReg);
    cc.and_(registers[instr.x()],instr.byte());
}

// Dxyn - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
void DRW(Opcode instr, const CPU &cpu,asmjit::x86::Gp cpubase, c8::Memory &mem,asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto simI = cc.newUIntPtr();
    auto simDisp = cc.newUIntPtr();
    auto loopCounter = cc.newUInt64();
    auto indexRegisterValue = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.xor_(registers[CPU::REG_F],registers[CPU::REG_F]);
    
    cc.mov(simDisp,reinterpret_cast<uint64_t>(cpu.display.data));
    cc.xor_(loopCounter,loopCounter);
    cc.mov(loopCounter.r8(),registers[instr.y()]);
    cc.shl(loopCounter,3);
    cc.and_(loopCounter,0xFF);
    
    auto test = cc.newUInt16();
    cc.xor_(test.r64(),test.r64());
    cc.mov(test,indexRegisterValue);
    cc.mov(simI,reinterpret_cast<uint64_t>(mem.memory.data()));
    cc.add(simI,test.r64());

    auto rem = cc.newUInt8();
    auto div = cc.newUInt64();
    auto div2 = cc.newUInt64();
    auto rem2 = cc.newUInt8();
    auto toPlace = cc.newUInt8();
    cc.xor_(div,div);
    cc.mov(rem,registers[instr.x()]);
    cc.mov(div.r8(),registers[instr.x()]);
    cc.and_(div,0x3F);
    cc.mov(rem,div.r8());

    cc.shr(div,3);
    cc.mov(div2,div);
    cc.inc(div2);
    cc.and_(div2.r64(),0x7);

    cc.and_(rem.r64(),0x7);
    cc.mov(rem2,8);
    cc.sub(rem2,rem);
    auto displayOffset = cc.newUIntPtr();
    auto flag = cc.newUInt8();
    for(int i=0;i<instr.low();i++)
    {   
        cc.mov(displayOffset,simDisp);
        cc.add(displayOffset,loopCounter);

        cc.mov(toPlace,asmjit::x86::ptr_8(simI,i));
        cc.shr(toPlace,rem);
        cc.test(asmjit::x86::ptr_8(displayOffset,div),toPlace);
        cc.setnz(flag);
        cc.xor_(asmjit::x86::ptr_8(displayOffset,div),toPlace);
        cc.or_(registers[CPU::REG_F],flag);

        cc.mov(toPlace,asmjit::x86::ptr_8(simI,i));
        cc.shl(toPlace,rem2);
        cc.test(asmjit::x86::ptr_8(displayOffset,div2),toPlace);
        cc.setnz(flag);
        cc.xor_(asmjit::x86::ptr_8(displayOffset,div2),toPlace);
        cc.or_(registers[CPU::REG_F],flag);
        cc.add(loopCounter,8);
        cc.and_(loopCounter,0xFF);
    }
}

// Ex9E - Skip instruction if key with the value of Vx is pressed.
    std::optional<asmjit::Label> SKP(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc,
                                     const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto butAddr = cc.newUIntPtr();
    cc.mov(butAddr,reinterpret_cast<uint64_t>(cpu.buttons));
    auto tmp = cc.newUIntPtr();
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r8(),registers[instr.x()]);
    cc.add(butAddr,tmp);
    asmjit::Label ljump = cc.newLabel();
    cc.cmp(asmjit::x86::ptr_8(butAddr),1);
    cc.je(ljump);
    return ljump;
}

// ExA1 - Skip instruction if key with the value of Vx is not pressed.
    std::optional<asmjit::Label> SKNP(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc,
                                      const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto butAddr = cc.newUIntPtr();
    cc.mov(butAddr,reinterpret_cast<uint64_t>(cpu.buttons));
    auto tmp = cc.newUIntPtr();
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r8(),registers[instr.x()]);
    cc.add(butAddr,tmp);
    asmjit::Label ljump = cc.newLabel();
    cc.cmp(asmjit::x86::ptr_8(butAddr),1);
    cc.jne(ljump);
    return ljump;
}

// Fx07 - Set Vx = delay timer value.
    void LD_VX_DT(Opcode instr, const asmjit::x86::Gp& cpubase, asmjit::x86::Compiler &cc,
                  const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto memreg = asmjit::x86::ptr_16(cpubase,offsetof(CPU, delayTimer));
    cc.mov(registers[instr.x()],memreg);
}

// Fx0A - Wait for a key press, store the value of the key in Vx.
    void LD_VX_K(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc,
                 const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto butAddr = cc.newUIntPtr();

    cc.mov(butAddr,reinterpret_cast<uint64_t>(cpu.buttons));
    cc.xor_(registers[instr.x()].r64(),registers[instr.x()].r64());

    asmjit::Label Loop = cc.newLabel(); 
    asmjit::Label Loop2 = cc.newLabel(); 
    cc.bind(Loop);
    for(int i = 0; i<16;i++){
        cc.mov(registers[instr.x()],i);
        cc.cmp(asmjit::x86::ptr_8(butAddr,i),1);
        cc.je(Loop2);
    }
    cc.jmp(Loop);

    auto EXIT = cc.newLabel();
    cc.bind(Loop2);
    cc.cmp(asmjit::x86::ptr_8(butAddr,registers[instr.x()].r64()),0);
    cc.je(EXIT);
    cc.jmp(Loop2);

    cc.bind(EXIT);
}

// Fx15 - Set delay timer = Vx.
    void LD_DT(Opcode instr, const asmjit::x86::Gp& cpubase, asmjit::x86::Compiler &cc,
               const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto memreg = asmjit::x86::ptr_16(cpubase,offsetof(CPU, delayTimer));
    cc.mov(memreg,registers[instr.x()]);
}

// Fx18 - Set sound timer = Vx.
    void LD_ST(Opcode instr, const asmjit::x86::Gp& cpubase, asmjit::x86::Compiler &cc,
               const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto memreg = asmjit::x86::ptr_16(cpubase,offsetof(CPU, soundTimer));
    cc.mov(memreg,registers[instr.x()]);
}

// Fx1E - Set I = I + Vx.
    void ADD_I_VX(Opcode instr, const asmjit::x86::Gp& cpubase, asmjit::x86::Compiler &cc,
                  const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto memreg = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    auto tmp = cc.newUInt16();
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r8(),registers[instr.x()]);
    cc.add(memreg,tmp);
}

// Fx29 - Set I = location of sprite for digit Vx.
    void LD_F_VX(Opcode instr, const asmjit::x86::Gp& cpubase, asmjit::x86::Compiler &cc,
                 const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto indexRegisterValue = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.mov(indexRegisterValue,0);
    cc.mov(indexRegisterValue,registers[instr.x()]);
    cc.shl(indexRegisterValue,2);
    cc.add(indexRegisterValue,registers[instr.x()]);
}

// Fx33 - Store BCD representation of Vx in memory locations I, I+1, and I+2.
    void
    LD_B_VX(Opcode instr, BasicBlock *bb, int pc, const asmjit::x86::Gp& cpubase, c8::Memory &mem, asmjit::x86::Compiler &cc,
            const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto simI = cc.newUIntPtr();
    auto indexRegisterValue = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.mov(simI,reinterpret_cast<uint64_t>(mem.getRawMemory()));
    auto tmp = cc.newUIntPtr();
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r16(),indexRegisterValue);
    cc.add(simI,tmp);

    auto startAddrT = cc.newUIntPtr();
    cc.mov(startAddrT,reinterpret_cast<uint64_t>(mem.getRawStartAddressTable()));

    auto divisor = cc.newUInt32();
    auto remainder = cc.newUInt32();
    auto tmpCalc = cc.newUIntPtr();
    cc.xor_(tmpCalc,tmpCalc);
    cc.mov(tmpCalc.r8(),registers[instr.x()]);
    auto overwrittenFunction = cc.newUInt16();
    for(int i=0;i<3;i++)
    {
        cc.xor_(remainder,remainder);
        cc.mov(divisor,10);
        cc.div(remainder,tmpCalc.r32(),divisor);
        cc.mov(asmjit::x86::ptr_8(simI,2-i),remainder.r8());
        
        cc.xor_(overwrittenFunction.r64(),overwrittenFunction.r64());
        cc.mov(overwrittenFunction.r16(), asmjit::x86::ptr_16(startAddrT,tmp,1));
        cc.mov(asmjit::x86::ptr_16(startAddrT,overwrittenFunction,1), 0);
        cc.inc(tmp);
    }
    //cc.add(indexRegisterValue,3);

    //Check if wrote into own block
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r16(),indexRegisterValue);
    auto ljump = cc.newLabel();
    //compares interval of writing (I, I + 3) to [pc+2, end]
    cc.cmp(tmp,(pc+2)-3);
    cc.jl(ljump);
    cc.cmp(tmp,bb->getEndAddr());
    cc.jg(ljump);

    
    bb->generateEpilogue(cc,cpubase,registers);
    auto retVal = cc.newUInt64();
    cc.xor_(retVal,retVal);
    cc.mov(retVal,tmp);
    cc.shl(retVal,16);
    cc.or_(retVal,0x8000 | ((pc)&0x0FFF));
    cc.ret(retVal);

    cc.bind(ljump);
    
}

// Fx55 - Store regs V0 through Vx in memory starting at location I.
    void
    LD_I_VX(Opcode instr, BasicBlock *bb, int pc, asmjit::x86::Gp cpubase, c8::Memory &mem, asmjit::x86::Compiler &cc,
            const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto simI = cc.newUIntPtr();
    auto indexRegisterValue = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.mov(simI,reinterpret_cast<uint64_t>(mem.getRawMemory()));

    auto startAddrT = cc.newUIntPtr();
    cc.mov(startAddrT,reinterpret_cast<uint64_t>(mem.getRawStartAddressTable()));

    auto tmp = cc.newUIntPtr();
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r16(),indexRegisterValue);
    cc.add(simI,tmp);

    auto overwrittenFunction = cc.newUInt16();
    cc.xor_(overwrittenFunction.r64(),overwrittenFunction.r64());
    for(int i=0;i<=instr.x();i++)
    {
        cc.mov(asmjit::x86::byte_ptr(simI,i),registers[i]);

        cc.xor_(overwrittenFunction.r64(),overwrittenFunction.r64());
        cc.mov(overwrittenFunction.r16(), asmjit::x86::ptr_16(startAddrT,tmp,1));
        cc.mov(asmjit::x86::ptr_16(startAddrT,overwrittenFunction,1), 0);
        cc.inc(tmp);
    }
    cc.add(indexRegisterValue,instr.x()+1);

    //Check if wrote into own block (that still needs to execute)
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r16(),indexRegisterValue);
    auto ljump = cc.newLabel();
    //compares interval of writing (I, I + {x}) to [pc+2, end]
    cc.cmp(tmp,(pc+2)-instr.x());
    cc.jl(ljump);
    cc.cmp(tmp,bb->getEndAddr());
    cc.jg(ljump);

    bb->generateEpilogue(cc,cpubase,registers);
    auto retVal = cc.newUInt64();
    cc.xor_(retVal,retVal);
    cc.mov(retVal,tmp);
    cc.shl(retVal,16);
    cc.or_(retVal,0x8000 | ((pc)&0x0FFF) );
    cc.ret(retVal);

    cc.bind(ljump);
}

// Fx65 - Read regs V0 through Vx from memory starting at location I.
    void LD_VX_I(Opcode instr, const asmjit::x86::Gp& cpubase, c8::Memory &mem, asmjit::x86::Compiler &cc,
                 const std::array<asmjit::x86::Gp, CPU::AMOUNT_REGISTERS> &registers) {
    auto simI = cc.newUIntPtr();
    auto indexRegisterValue = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.mov(simI,reinterpret_cast<uint64_t>(mem.getRawMemory()));
    auto tmp = cc.newUInt64();
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r16(),indexRegisterValue);
    cc.add(simI,tmp);
    for(int i=0;i<=instr.x();i++)
    {
        cc.mov(registers[i],asmjit::x86::byte_ptr(simI,i));
    }
    cc.add(indexRegisterValue,instr.x()+1);
}

}

#endif  // CHIP8_INSTRUCTIONS_H

