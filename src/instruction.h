
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
void CLS(Opcode instr, const CPU &cpu, BasicBlock* basicBlock ,asmjit::x86::Compiler &cc,asmjit::x86::Gp cpubase, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    auto simDisp = cc.newUIntPtr();
    cc.mov(simDisp,reinterpret_cast<uint64_t>(cpu.display.data));
    for(int i=0;i<sizeof(cpu.display.data)/(64/8);i++)
    {
        cc.mov(asmjit::x86::ptr_64(simDisp,i*(64/8)),0);
    }

}

// 00EE - Return from a subroutine.
void RET(Opcode instr, const CPU &cpu, BasicBlock* basicBlock ,asmjit::x86::Compiler &cc,asmjit::x86::Gp cpubase, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
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
    auto val = cc.newUInt16();
    cc.mov(val, asmjit::x86::ptr_16(ptr));
    cc.ret(val);
}

// 1nnn - Jump to location nnn.
void JMP(Opcode instr, BasicBlock* basicBlock ,asmjit::x86::Compiler &cc,asmjit::x86::Gp cpubase, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers)
{
    basicBlock->generateEpilogue(cc,cpubase,registers);
    auto retVal = cc.newUInt16();
    cc.mov(retVal,instr.address());
    cc.ret(retVal);
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
    basicBlock->generateEpilogue(cc,cpubase,registers);
    auto retVal = cc.newUInt16();
    cc.mov(retVal,instr.address());
    cc.ret(retVal);
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
    cc.setc(registers[CPU::REG_F]);
}

// 8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow.
void SUBN_VX_VY(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto tmp = cc.newUInt8();
    cc.mov(tmp,registers[instr.y()]);
    cc.sub(tmp,registers[instr.x()]);
    cc.mov(registers[instr.x()],tmp);
    cc.setnc(registers[CPU::REG_F]);
}

// 8xyE - Set Vx = Vx SHL 1.
void SHL_VX(Opcode instr, const CPU &cpu, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    cc.shl(registers[instr.x()],1);
    cc.setc(registers[15]);
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
    basicBlock->generateEpilogue(cc,cpubase,registers);
    auto retVal = cc.newUInt16();
    cc.xor_(retVal,retVal);
    cc.mov(retVal.r8(),registers[0]);
    cc.add(retVal,instr.address());
    cc.ret(retVal);
}

// Cxkk - Set Vx = random byte AND kk
void RND(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
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
    cc.mov(test.r64(),0);
    cc.mov(test,indexRegisterValue);
    cc.mov(simI,reinterpret_cast<uint64_t>(mem.memory.data()));
    cc.add(simI,test.r64());

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
std::optional<asmjit::Label> SKP(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto button = asmjit::x86::ptr_8(cpubase,offsetof(CPU, buttons));
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
std::optional<asmjit::Label> SKNP(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto button = asmjit::x86::ptr_8(cpubase,offsetof(CPU, buttons));
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
void LD_VX_DT(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers){
    auto memreg = asmjit::x86::ptr_16(cpubase,offsetof(CPU, delayTimer));
    cc.mov(registers[instr.x()],memreg);
}

// Fx0A - Wait for a key press, store the value of the key in Vx.
void LD_VX_K(Opcode instr, const CPU &cpu, asmjit::x86::Gp cpubase, asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto button = asmjit::x86::ptr_8(cpubase,offsetof(CPU, buttons));
    auto butAddr = cc.newUIntPtr();
    cc.mov(butAddr,reinterpret_cast<uint64_t>(cpu.buttons));

    asmjit::Label Loop = cc.newLabel(); 
    asmjit::Label Exit = cc.newLabel();
    cc.bind(Loop);
    for(int i = 0; i<16;i++){
        cc.mov(registers[instr.x()],i);
        cc.cmp(asmjit::x86::ptr_8(butAddr,i),0);
        cc.jne(Exit);
    }
    cc.jmp(Loop);
    cc.bind(Exit);
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
    auto tmp = cc.newUInt16();
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r8(),registers[instr.x()]);
    cc.add(memreg,tmp);
}

// Fx29 - Set I = location of sprite for digit Vx.
void LD_F_VX(Opcode instr, const CPU &cpu,asmjit::x86::Gp cpubase, c8::Memory &mem,asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto indexRegisterValue = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.mov(indexRegisterValue,0);
    cc.mov(indexRegisterValue,registers[instr.x()]);
    cc.shl(indexRegisterValue,2);
    cc.add(indexRegisterValue,registers[instr.x()]);
}

// Fx33 - Store BCD representation of Vx in memory locations I, I+1, and I+2.
void LD_B_VX(Opcode instr, const CPU &cpu,asmjit::x86::Gp cpubase, c8::Memory &mem,asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto indexRegisterValue = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    
    auto divisor = cc.newUInt32();
    auto remainder = cc.newUInt32();
    auto memPtr = cc.newUIntPtr();
    cc.mov(memPtr,reinterpret_cast<uint64_t>(mem.getRawMemory()));
    auto tmp = cc.newUIntPtr();
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r8(),registers[instr.x()]);
    cc.add(memPtr,tmp);
    for(int i=0;i<3;i++)
    {
        cc.xor_(remainder,remainder);
        cc.mov(divisor,10);
        cc.div(remainder,tmp.r32(),divisor);
        cc.mov(asmjit::x86::ptr_8(memPtr,2-i),remainder.r8());
    }
}

// Fx55 - Store regs V0 through Vx in memory starting at location I.
void LD_I_VX(Opcode instr, const CPU &cpu,asmjit::x86::Gp cpubase, c8::Memory &mem,asmjit::x86::Compiler &cc, const std::array<asmjit::x86::Gp,CPU::AMOUNT_REGISTERS> &registers) {
    auto simI = cc.newUIntPtr();
    auto indexRegisterValue = asmjit::x86::ptr_16(cpubase,offsetof(CPU, indexRegister));
    cc.mov(simI,reinterpret_cast<uint64_t>(mem.getRawMemory()));
    auto tmp = cc.newUIntPtr();
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r16(),indexRegisterValue);
    cc.add(simI,tmp);
    for(int i=0;i<=instr.x();i++)
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
    auto tmp = cc.newUInt64();
    cc.xor_(tmp,tmp);
    cc.mov(tmp.r16(),indexRegisterValue);
    cc.add(simI,tmp);
    for(int i=0;i<=instr.x();i++)
    {
        cc.mov(registers[i],asmjit::x86::byte_ptr(simI,i));
    }
}

}

#endif  // CHIP8_INSTRUCTIONS_H

