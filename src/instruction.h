
#ifndef CHIP8_INSTRUCTIONS_H
#define CHIP8_INSTRUCTIONS_H

#include <asmjit/asmjit.h>
#include "cpu.h"
#include "types.h"
#include "memory.h"
#include <optional>
#include<cstdlib>

namespace c8::instruction {

// 00E0 - Clear the display.


// 00EE - Return from a subroutine.
void RET(Opcode instr, const CPU &cpu, BasicBlock* basicBlock ,asmjit::x86::Compiler &cc,asmjit::x86::Gp cpubase, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    auto ptr = cc.newUIntPtr();
    auto stackPointer = asmjit::x86::ptr_16(cpubase,offsetof(CPU, sp));
    cc.dec(stackPointer);
    cc.mov(ptr,&cpu.stack);
    cc.add(ptr,stackPointer);
    cc.add(ptr,stackPointer);
    basicBlock->generateEpilogue(cc,cpubase,registers);
    cc.mov(asmjit::x86::rax, asmjit::x86::ptr_16(ptr));
    cc.ret();
}

// 1nnn - Jump to location nnn.
void JMP(Opcode instr, BasicBlock* basicBlock ,asmjit::x86::Compiler &cc,asmjit::x86::Gp cpubase, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    basicBlock->generateEpilogue(cc,cpubase,registers);
    cc.mov(asmjit::x86::rax,instr.address());
    cc.ret();
}

// 2nnn - Call subroutine at nnn.
void CALL(Opcode instr, const CPU &cpu, BasicBlock* basicBlock, uint16_t pc ,asmjit::x86::Compiler &cc,asmjit::x86::Gp cpubase, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    auto ptr = cc.newUIntPtr();
    auto stackPointer = asmjit::x86::ptr_16(cpubase,offsetof(CPU, sp));
    cc.mov(ptr,&cpu.stack);
    cc.add(ptr,stackPointer);
    cc.add(ptr,stackPointer);
    cc.mov(asmjit::x86::ptr_16(ptr), pc+2);
    cc.inc(stackPointer);
    basicBlock->generateEpilogue(cc,cpubase,registers);
    cc.mov(asmjit::x86::rax, instr.address());
    cc.ret();
}
// 3xkk - Skip next instruction if Vx = kk.
std::optional<asmjit::Label> SE_VX_KK(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    asmjit::Label ljump = cc.newLabel();
    cc.cmp(registers[instr.x()],instr.byte());
    cc.je(ljump);
    return ljump;
}

// 4xkk - Skip next instruction if Vx != kk.
std::optional<asmjit::Label> SNE_VX_KK(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    asmjit::Label ljump = cc.newLabel();
    cc.cmp(registers[instr.x()],instr.byte());
    cc.jne(ljump);
    return ljump;
}

// 5xy0 - Skip next instruction if Vx = Vy.
std::optional<asmjit::Label> SE_VX_VY(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    asmjit::Label ljump = cc.newLabel();
    cc.cmp(registers[instr.x()],registers[instr.y()]);
    cc.je(ljump);
    return ljump;
}
// 6xkk - The interpreter puts the value kk into register Vx.
void LD_VX_KK(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.mov(registers[instr.x()],instr.byte());
}

// 7xkk - Adds the value kk to the value of register Vx.
void ADD_VX_KK(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.add(registers[instr.x()],instr.byte());
}

// 8xy0 - Stores the value of register Vy in register Vx.
void LD_VX_VY(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.mov(registers[instr.x()],registers[instr.y()]);
}

// 8xy1 - Performs a bitwise OR on the values of Vx and Vy.
void OR_VX_VY(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.or_(registers[instr.x()],registers[instr.y()]);
}

// 8xy2 - Performs a bitwise AND on the values of Vx and Vy.
void AND_VX_VY(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.and_(registers[instr.x()],registers[instr.y()]);
}

// 8xy3 - Performs a bitwise exclusive OR on the values of Vx and Vy.
void XOR_VX_VY(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.xor_(registers[instr.x()],registers[instr.y()]);
}

// 8xy4 - Set Vx = Vx + Vy, set VF = carry.
void ADD_VX_VY(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.add(registers[instr.x()],registers[instr.y()]);
    cc.setc(registers[CPU::REG_F]);
}

// 8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.
void SUB_VX_VY(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.sub(registers[instr.x()],registers[instr.y()]);
    cc.setnc(registers[CPU::REG_F]);
}

// 8xy6 - Set Vx = Vx SHR 1.
void SHR_VX(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.shr(registers[instr.x()],1);
}

// 8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow.
void SUBN_VX_VY(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.sub(registers[instr.x()],registers[instr.y()]);
    cc.setc(registers[CPU::REG_F]);
    cc.neg(registers[instr.x()]);
}

// 8xyE - Set Vx = Vx SHL 1.
void SHL_VX(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.shl(registers[instr.x()],1);
}

// 9xy0 - Skip next instruction if Vx != Vy.
std::optional<asmjit::Label> SNE_VX_VY(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    asmjit::Label ljump = cc.newLabel();
    cc.cmp(registers[instr.x()],registers[instr.y()]);
    cc.jne(ljump);
    return ljump;
}

// Annn - Register I is set to nnn.
void LD_I(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto memreg = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.mov(memreg,instr.address());
}

// Bnnn - Program counter is set to nnn plus the value of V0.
void JMP_V0(Opcode instr, const CPU &cpu, BasicBlock* basicBlock ,asmjit::x86::Compiler &cc,asmjit::x86::Gp cpubase, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.mov(asmjit::x86::rax,registers[0]);
    cc.add(asmjit::x86::rax,instr.address());
    basicBlock->generateEpilogue(cc,cpubase,registers);
    cc.endFunc();
}

// Cxkk - Set Vx = random byte AND kk
void RND(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto savedreg = cc.newUInt8();
    cc.push(asmjit::x86::rax);
    cc.push(asmjit::x86::rdx);
    cc.push(asmjit::x86::rcx);
    cc.push(asmjit::x86::r8);
    cc.push(asmjit::x86::r9);
    cc.push(asmjit::x86::r10);
    cc.push(asmjit::x86::r11);
    cc.call(asmjit::Imm(rand));
    cc.mov(savedreg,asmjit::x86::al);
    cc.pop(asmjit::x86::r11);
    cc.pop(asmjit::x86::r10);
    cc.pop(asmjit::x86::r9);
    cc.pop(asmjit::x86::r8);
    cc.pop(asmjit::x86::rcx);
    cc.pop(asmjit::x86::rdx);
    cc.mov(registers[instr.x()],savedreg);
    cc.pop(asmjit::x86::rax);
    cc.and_(registers[instr.x()],instr.byte());
    savedreg.reset();
}

// Dxyn - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
void DRW(Opcode instr, const CPU &cpu,asmjit::x86::Gp cpubase, c8::Memory &mem,asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto simI = cc.newUIntPtr();
    auto simDisp = cc.newUIntPtr();
    auto loopCounter = cc.newUInt64();
    auto indexRegisterValue = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));

    cc.mov(simDisp,reinterpret_cast<uint64_t>(cpu.display.data));
    cc.mov(loopCounter.r8(),registers[instr.y()]);
    cc.and_(loopCounter,0x1F);
    cc.shl(loopCounter,3);

    cc.mov(simI,reinterpret_cast<uint64_t>(&mem.memory.data()[0]));
    cc.add(simI,indexRegisterValue);
    
    
    auto rem = cc.newUInt8();
    auto div = cc.newUInt64();
    auto div2 = cc.newUInt64();
    auto rem2 = cc.newUInt8();
    auto toPlace = cc.newUInt8();

    cc.mov(rem,registers[instr.x()]);
    cc.mov(div.r8(),registers[instr.x()]);
    cc.and_(div,0x3F);
    cc.and_(rem,0x3F);
    cc.shr(div,3);
    cc.mov(div2,div);
    cc.inc(div2);
    cc.and_(div2.r64(),0x7);
    cc.and_(rem.r64(),0x7);
    cc.mov(rem2,8);
    cc.sub(rem2,rem);
    auto displayOffset = cc.newUIntPtr();
    for(int i=0;i<instr.low();i++)
    {   
        cc.mov(displayOffset,simDisp);
        cc.add(displayOffset,loopCounter);

        cc.mov(toPlace,asmjit::x86::ptr_8(simI));
        cc.shr(toPlace,rem);
        cc.xor_(asmjit::x86::ptr_8(displayOffset,div),toPlace);
        
        cc.mov(toPlace,asmjit::x86::ptr_8(simI));
        cc.shl(toPlace,rem2);
        cc.xor_(asmjit::x86::ptr_8(displayOffset,div2),toPlace);
        cc.add(loopCounter,8);
        cc.and_(loopCounter,256-1);
        cc.inc(simI);
    }
}

// Ex9E - Skip instruction if key with the value of Vx is pressed.
void SKP(Opcode in, Cpu *cpu) {
}

// ExA1 - Skip instruction if key with the value of Vx is not pressed.
void SKNP(Opcode in, Cpu *cpu) {
}

// Fx07 - Set Vx = delay timer value.
void LD_VX_DT(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers){
    auto memreg = asmjit::x86::ptr_16(cpubase,offsetof(CPU, delayTimer));
    cc.mov(registers[instr.x()],memreg);
}

// Fx0A - Wait for a key press, store the value of the key in Vx.
void LD_VX_K(Opcode in, Cpu *cpu) {
}

// Fx15 - Set delay timer = Vx.
void LD_DT(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto memreg = asmjit::x86::ptr_16(cpubase,offsetof(CPU, delayTimer));
    cc.mov(memreg,registers[instr.x()]);
}

// Fx18 - Set sound timer = Vx.
void LD_ST(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto memreg = asmjit::x86::ptr_16(cpubase,offsetof(CPU, soundTimer));
    cc.mov(memreg,registers[instr.x()]);
}

// Fx1E - Set I = I + Vx.
void ADD_I_VX(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto memreg = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.add(memreg,registers[instr.x()]);
}

// Fx29 - Set I = location of sprite for digit Vx.
void LD_F_VX(Opcode in, Cpu *cpu) {
}

// Fx33 - Store BCD representation of Vx in memory locations I, I+1, and I+2.
void LD_B_VX(Opcode in, Cpu *cpu) {
}

// Fx55 - Store regs V0 through Vx in memory starting at location I.
void LD_I_VX(Opcode instr, const CPU &cpu,asmjit::x86::Gp cpubase, c8::Memory &mem,asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto simI = cc.newUIntPtr();
    auto indexRegisterValue = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.mov(simI,reinterpret_cast<uint64_t>(mem.getRawMemory()));
    cc.add(simI,indexRegisterValue);
    for(int i=0;i<instr.x();i++)
    {
        cc.mov(asmjit::x86::byte_ptr(simI,i),registers[i]);
    }
    simI.reset();
}

// Fx65 - Read regs V0 through Vx from memory starting at location I.
void LD_VX_I(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, c8::Memory &mem,asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto simI = cc.newUIntPtr();
    auto indexRegisterValue = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.mov(simI,reinterpret_cast<uint64_t>(mem.getRawMemory()));
    cc.add(simI,indexRegisterValue);
    for(int i=0;i<instr.x();i++)
    {
        cc.mov(registers[i],asmjit::x86::byte_ptr(simI,i));
    }
}

}

#endif  // CHIP8_INSTRUCTIONS_H