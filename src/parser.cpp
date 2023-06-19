#include "parser.h"

// Standard Chip-8 instructions reference:
// http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.1
Instruction Parser::parse(c8::Opcode opcode) {
    switch (opcode.high()) {
        case 0x00:
            switch (opcode.byte()) {
                // 00E0 - Clear the display.
                case 0xE0: return Instruction::CLS;
                // 00EE - Return from a subroutine.
                case 0xEE: return Instruction::RET;
                // UNKNOWN
                default: return Instruction::UNKNOWN;
            }
        // 1nnn - Jump to location nnn.
        case 0x01: return Instruction::JMP;
        // 2nnn - Call subroutine at nnn.
        case 0x02: return Instruction::CALL;
        // 3xkk - Skip next instruction if Vx = kk.
        case 0x03: return Instruction::SE_VX_KK;
        // 4xkk - Skip next instruction if Vx != kk.
        case 0x04: return Instruction::SNE_VX_KK;
        // 5xy0 - Skip next instruction if Vx = Vy.
        case 0x05: return Instruction::SE_VX_VY;
        // 6xkk - The interpreter puts the value kk into register Vx.
        case 0x06: return Instruction::LD_VX_KK;
        // 7xkk - Adds the value kk to the value of register Vx.
        case 0x07: return Instruction::ADD_VX_KK;
        case 0x08:
            switch (opcode.low()) {
                // 8xy0 - Stores the value of register Vy in register Vx.
                case 0x00: return Instruction::LD_VX_VY;
                // 8xy1 - Performs a bitwise OR on the values of Vx and Vy.
                case 0x01: return Instruction::OR_VX_VY;
                // 8xy2 - Performs a bitwise AND on the values of Vx and Vy.
                case 0x02: return Instruction::AND_VX_VY;
                // 8xy3 - Performs a bitwise exclusive OR on the values of Vx and Vy.
                case 0x03: return Instruction::XOR_VX_VY;
                // 8xy4 - Set Vx = Vx + Vy, set VF = carry.
                case 0x04: return Instruction::ADD_VX_VY;
                // 8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.
                case 0x05: return Instruction::SUB_VX_VY;
                // 8xy6 - Set Vx = Vx SHR 1.
                case 0x06: return Instruction::SHR_VX;
                // 8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow.
                case 0x07: return Instruction::SUBN_VX_VY;
                // 8xyE - Set Vx = Vx SHL 1.
                case 0x0E: return Instruction::SHL_VX;
                // UNKNOWN
                default: return Instruction::UNKNOWN;
            }
        // 9xy0 - Skip next instruction if Vx != Vy.
        case 0x09: return Instruction::SNE_VX_VY;
        // Annn - Register I is set to nnn.
        case 0x0A: return Instruction::LD_I;
        // Bnnn - Program counter is set to nnn plus the value of V0.
        case 0x0B: return Instruction::JMP_V0;
        // Cxkk - Set Vx = random byte AND kk.
        case 0x0C: return Instruction::RND;
        // Dxyn - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        case 0x0D: return Instruction::DRW;
        case 0x0E:
            switch (opcode.byte()) {
                // Ex9E - Skip instruction if key with the value of Vx is pressed.
                case 0x9E: return Instruction::SKP;
                // ExA1 - Skip instruction if key with the value of Vx is not pressed.
                case 0xA1: return Instruction::SKNP;
                // UNKNOWN
                default: return Instruction::UNKNOWN;
            }
        case 0x0F:
            switch (opcode.byte()) {
                // Fx07 - Set Vx = delay timer value.
                case 0x07: return Instruction::LD_VX_DT;
                // Fx0A - Wait for a key press, store the value of the key in Vx.
                case 0x0A: return Instruction::LD_VX_K;
                // Fx15 - Set delay timer = Vx.
                case 0x15: return Instruction::LD_DT;
                // Fx18 - Set sound timer = Vx.
                case 0x18: return Instruction::LD_ST;
                // Fx1E - Set I = I + Vx.
                case 0x1E: return Instruction::ADD_I_VX;
                // Fx29 - Set I = location of sprite for digit Vx.
                case 0x29: return Instruction::LD_F_VX;
                // Fx33 - Store BCD representation of Vx in memory locations I, I+1, and I+2.
                case 0x33: return Instruction::LD_B_VX;
                // Fx55 - Store registers V0 through Vx in memory starting at location I.
                case 0x55: return Instruction::LD_I_VX;
                // Fx65 - Read registers V0 through Vx from memory starting at location I.
                case 0x65: return Instruction::LD_VX_I;
                // UNKNOWN
                default: return Instruction::UNKNOWN;
            }
        default: return Instruction::UNKNOWN;
    }   
}

std::bitset<CPU::AMOUNT_REGISTERS> Parser::parseUsedRegisters(c8::Opcode opcode) {
    switch (opcode.high()) {
        case 0x00:
            switch (opcode.byte()) {
                // 00E0 - Clear the display.
                case 0xE0: return std::bitset<CPU::AMOUNT_REGISTERS>(0);
                // 00EE - Return from a subroutine.
                case 0xEE: return std::bitset<CPU::AMOUNT_REGISTERS>(0);
                // UNKNOWN
                default: return std::bitset<CPU::AMOUNT_REGISTERS>(0);
            }
        // 1nnn - Jump to location nnn.
        case 0x01: return std::bitset<CPU::AMOUNT_REGISTERS>(0);
        // 2nnn - Call subroutine at nnn.
        case 0x02: return std::bitset<CPU::AMOUNT_REGISTERS>(0);
        // 3xkk - Skip next instruction if Vx = kk.
        case 0x03: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
        // 4xkk - Skip next instruction if Vx != kk.
        case 0x04: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
        // 5xy0 - Skip next instruction if Vx = Vy.
        case 0x05: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x() | 1<<opcode.y());
        // 6xkk - The interpreter puts the value kk into register Vx.
        case 0x06: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
        // 7xkk - Adds the value kk to the value of register Vx.
        case 0x07: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
        case 0x08:
            switch (opcode.low()) {
                // 8xy0 - Stores the value of register Vy in register Vx.
                case 0x00: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x()| 1<<opcode.y());
                // 8xy1 - Performs a bitwise OR on the values of Vx and Vy.
                case 0x01: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x()| 1<<opcode.y());
                // 8xy2 - Performs a bitwise AND on the values of Vx and Vy.
                case 0x02: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x()| 1<<opcode.y());
                // 8xy3 - Performs a bitwise exclusive OR on the values of Vx and Vy.
                case 0x03: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x()| 1<<opcode.y());
                // 8xy4 - Set Vx = Vx + Vy, set VF = carry.
                case 0x04: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x()| 1<<opcode.y() | 0x8000);
                // 8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.
                case 0x05: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x()| 1<<opcode.y() | 0x8000);
                // 8xy6 - Set Vx = Vx SHR 1.
                case 0x06: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x() |  1<<opcode.y() | 0x8000);
                // 8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow.
                case 0x07: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x()| 1<<opcode.y() | 0x8000);
                // 8xyE - Set Vx = Vy SHL 1.
                case 0x0E: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x() | 1<<opcode.y()  | 0x8000);
                // UNKNOWN
                default: return std::bitset<CPU::AMOUNT_REGISTERS>(0);
            }
        // 9xy0 - Skip next instruction if Vx != Vy.
        case 0x09: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x()| 1<<opcode.y());
        // Annn - Register I is set to nnn.
        case 0x0A: return std::bitset<CPU::AMOUNT_REGISTERS>(0);
        // Bnnn - Program counter is set to nnn plus the value of V0.
        case 0x0B: return std::bitset<CPU::AMOUNT_REGISTERS>(1);
        // Cxkk - Set Vx = random byte AND kk.
        case 0x0C: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
        // Dxyn - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        case 0x0D: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x()| 1<<opcode.y() | 0x8000);
        case 0x0E:
            switch (opcode.byte()) {
                // Ex9E - Skip instruction if key with the value of Vx is pressed.
                case 0x9E: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
                // ExA1 - Skip instruction if key with the value of Vx is not pressed.
                case 0xA1: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
                // UNKNOWN
                default: return std::bitset<CPU::AMOUNT_REGISTERS>(0);
            }
        case 0x0F:
            switch (opcode.byte()) {
                // Fx07 - Set Vx = delay timer value.
                case 0x07: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
                // Fx0A - Wait for a key press, store the value of the key in Vx.
                case 0x0A: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
                // Fx15 - Set delay timer = Vx.
                case 0x15: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
                // Fx18 - Set sound timer = Vx.
                case 0x18: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
                // Fx1E - Set I = I + Vx.
                case 0x1E: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
                // Fx29 - Set I = location of sprite for digit Vx.
                case 0x29: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
                // Fx33 - Store BCD representation of Vx in memory locations I, I+1, and I+2.
                case 0x33: return std::bitset<CPU::AMOUNT_REGISTERS>(1<<opcode.x());
                // Fx55 - Store registers V0 through Vx in memory starting at location I.
                case 0x55: return std::bitset<CPU::AMOUNT_REGISTERS>((1<<(opcode.x()+1))-1);
                // Fx65 - Read registers V0 through Vx from memory starting at location I.
                case 0x65: return std::bitset<CPU::AMOUNT_REGISTERS>((1<<(opcode.x()+1))-1);
                // UNKNOWN
                default: return std::bitset<CPU::AMOUNT_REGISTERS>(0);
            }
        default: return std::bitset<CPU::AMOUNT_REGISTERS>(0);
    }   
}

bool Parser::isJumpInstruction(Instruction instr , std::optional<Instruction> before) {
    if(before.has_value()){
        return !(before == Instruction::SKNP ||
                before == Instruction::SKP  ||
                before == Instruction::SE_VX_KK ||
                before == Instruction::SE_VX_VY ||
                before == Instruction::SNE_VX_KK ||
                before == Instruction:: SNE_VX_VY 
               ) && 
               (instr == Instruction::JMP_V0 ||
                   instr == Instruction::JMP ||
                   instr == Instruction::RET ||
                   instr == Instruction::CALL);
    } else {
        return (instr == Instruction::JMP_V0 ||
                   instr == Instruction::JMP ||
                   instr == Instruction::RET ||
                   instr == Instruction::CALL);
    }
}

bool Parser::isWriteInstruction(Instruction instr ) {
    return (instr == Instruction::LD_B_VX ||
            instr == Instruction::LD_I_VX);
}