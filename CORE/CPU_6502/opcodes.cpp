#include "core.h"
#include <cstdint>
#include <iostream>
#include <sys/types.h>

using namespace std;

// OFFICIAL INSTRUCTIONS

// Add withn carry, adds the carry flag and a memory value to the accumulator
// carry flag is the set to the carry value coming out of bit 7
void CPU_6502::ADC(uint16_t addr) {

  const uint16_t value = read(addr);
  uint16_t sum = A + value + (GET_FLAG(CARRY) ? 1 : 0);
  SET_FLAG(CARRY, sum > ZERO_PAGE_MASK);
  SET_FLAG(ZERO, (sum & ZERO_PAGE_MASK) == 0);
  SET_FLAG(OVERFLOW, (~(A ^ value) & (A ^ sum) & 0x80) != 0);
  SET_FLAG(NEGATIVE, sum & 0x80);
  A = (sum & ZERO_PAGE_MASK);
}

// Bitwise AND, ANDs a memory value with the accumulator, bit by bit
void CPU_6502::AND(uint16_t addr) {

  A &= read(addr);
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

// Arithmetic shift left, shifts all bits of a value one position to the left
// bit 7 is shifted into the carry flag, and 0 is shifted into bit 0
// equivalent to multiplying the unsigned value by 2
void CPU_6502::ASL(uint16_t addr) {

  if (LOOKUP[read(PC - 1)].addr_mode == &CPU_6502::ACCUMULATOR) {
    SET_FLAG(CARRY, A & 0x80);
    A <<= 1;
    SET_FLAG(ZERO, A == 0);
    SET_FLAG(NEGATIVE, A & 0x80);
  } else {
    uint8_t data = read(addr);
    SET_FLAG(CARRY, data & 0x80);
    data <<= 1;
    write(addr, data);
    SET_FLAG(ZERO, data == 0);
    SET_FLAG(NEGATIVE, data & 0x80);
  }
}

// Branch if carry clear, if the carry is clear then branches by adding the
// offset to the PC, offset is signed and has a range of -128 -> 127
void CPU_6502::BCC(uint16_t addr) { BRANCH(addr, !GET_FLAG(CARRY)); }

// Branch if carry set, if the carry is set then branches by adding the offset
// to the PC, offset is signed and has a range of -128 -> 127
void CPU_6502::BCS(uint16_t addr) {
  // cout << " CARRY: " << to_string(GET_FLAG(CARRY)) << endl;

  BRANCH(addr, GET_FLAG(CARRY));
}

// Branch if equal, if the zero flag is set then branches by adding the offset
// to the PC, offset is signed and has a range of -128 -> 127
void CPU_6502::BEQ(uint16_t addr) { BRANCH(addr, GET_FLAG(ZERO)); }

// Branch if minus, if the negative flag is set then branches by adding the
// offset to the PC, offset is signed and has a range of -128 -> 127
void CPU_6502::BMI(uint16_t addr) { BRANCH(addr, GET_FLAG(NEGATIVE)); }

// Branch if not equal, if the zero flag is clear then branches by adding the
// relative offset to the PC, offset is signed and has a range of -128 -> 127
void CPU_6502::BNE(uint16_t addr) { BRANCH(addr, !GET_FLAG(ZERO)); }

// Branch if plus, if the negative flag is clear then branches by adding the
// relative offset to the PC, offset is signed and has a range of -128 -> 127
void CPU_6502::BPL(uint16_t addr) { BRANCH(addr, !GET_FLAG(NEGATIVE)); }

// Branch if overflow clear, if the overflow flag is clear then branches by
// adding the relative offset to the PC, offset is signed and has a range of
// -128
// -> 127
void CPU_6502::BVC(uint16_t addr) { BRANCH(addr, !GET_FLAG(OVERFLOW)); }

// Branch if overflow set, if the overflow flag is set then branches by adding
// the relative offset to the PC, offset is signed and has a range of -128 ->
// 127
void CPU_6502::BVS(uint16_t addr) { BRANCH(addr, GET_FLAG(OVERFLOW)); }

// Bit test, modifies flags but does not change memory or registers.
// the zero flag is set depending on the reselt of the accumulator AND memory
// value bits 7 and 6 of the memory value are loaded directly into the negative
// and overflow flags
void CPU_6502::BIT(uint16_t addr) {
  const uint8_t data = read(addr);
  SET_FLAG(ZERO, (A & data) == 0);
  SET_FLAG(OVERFLOW, data & 0x40);
  SET_FLAG(NEGATIVE, data & 0x80);
}

// Break, software IRQ (interrupt request), triggers an IRQ
// this instruction is the only way to trigger an IRQ via software
// pushes the current PC and flags to the stack, sets the IRQ flag, and jumps to
// the IRQ handler unlike a normal IRQ, sets the break flag in the flags byte
// and triggers an interrupt even if the interrupt disable flag is set the
// return address that is pushed to the stack skips the byte after the BRK
// opcode
void CPU_6502::BRK(uint16_t addr) {
  PC++;
  push((PC >> 8) & 0xFF);
  push(PC & 0xFF);
  SET_FLAG(B_FLAG, true);
  SET_FLAG(UNUSED, true);
  push(STATUS_REGISTER);
  SET_FLAG(INTERRUPT_DISABLE, true);
  uint16_t lo = read(IRQ_VECTOR);
  uint16_t hi = read(IRQ_VECTOR + 1);
  PC = (hi << 8) | lo;
}

// Clear carry, clears the carry flag
void CPU_6502::CLC(uint16_t addr) {
  SET_FLAG(CARRY, false);

  //  cout << " CARRY: " << to_string(GET_FLAG(CARRY)) << endl;
}

// Clear decimal, clears the decimal flag
void CPU_6502::CLD(uint16_t addr) { SET_FLAG(DECIMAL, false); }

// Clear interrupt disable, clears the interrupt disable flag
void CPU_6502::CLI(uint16_t addr) { SET_FLAG(INTERRUPT_DISABLE, false); }

// Clear overflow, clears the overflow flag
void CPU_6502::CLV(uint16_t addr) { SET_FLAG(OVERFLOW, false); }

// Compare A, compares the accumulator to a memory value
// sets flags as appropriate but does not modify any of the registers
// comparison is implemented as a subtraction, setting carry if there is no
// borrow zero if the result is 0, negative if the result is negative
void CPU_6502::CMP(uint16_t addr) {
  const uint8_t data = read(addr);
  uint16_t diff = A - data;
  SET_FLAG(CARRY, A >= data);
  SET_FLAG(ZERO, (diff & ZERO_PAGE_MASK) == 0);
  SET_FLAG(NEGATIVE, diff & 0x80);
}

// Compare X, compares X to a memory value
// sets flags as appropriate but does not modify any of the registers
// comparison is implemented as a subtraction, setting carry if there is no
// borrow zero if the result is 0, negative if the result is negative
void CPU_6502::CPX(uint16_t addr) {
  const uint8_t data = read(addr);
  uint16_t diff = X - data;
  SET_FLAG(CARRY, X >= data);
  SET_FLAG(ZERO, (diff & ZERO_PAGE_MASK) == 0);
  SET_FLAG(NEGATIVE, diff & 0x80);
}

// Compare Y, compares Y to a memory value
// sets flags as appropriate but does not modify any of the registers
// comparison is implemented as a subtraction, setting carry if there is no
// borrow zero if the result is 0, negative if the result is negative
void CPU_6502::CPY(uint16_t addr) {
  const uint8_t data = read(addr);
  uint16_t diff = Y - data;
  SET_FLAG(CARRY, Y >= data);
  SET_FLAG(ZERO, (diff & ZERO_PAGE_MASK) == 0);
  SET_FLAG(NEGATIVE, diff & 0x80);
}

// Decrement memory, subtracts 1 from a memory location
void CPU_6502::DEC(uint16_t addr) {
  uint8_t data = read(addr) - 1;
  write(addr, data);
  SET_FLAG(ZERO, data == 0);
  SET_FLAG(NEGATIVE, data & 0x80);
}

// Decrement X, subtracts 1 from the X register
void CPU_6502::DEX(uint16_t addr) {
  X--;
  SET_FLAG(ZERO, X == 0);
  SET_FLAG(NEGATIVE, X & 0x80);
}

// Decrement Y, subtracts 1 from the Y register
void CPU_6502::DEY(uint16_t addr) {
  Y--;
  SET_FLAG(ZERO, Y == 0);
  SET_FLAG(NEGATIVE, Y & 0x80);
}

// Bitwise exclusive OR, XORs a memory value with the accumulator, bit by bit
void CPU_6502::EOR(uint16_t addr) {
  A ^= read(addr);
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

// Increment memory, 1 is added to a memory value
void CPU_6502::INC(uint16_t addr) {
  uint8_t data = read(addr) + 1;
  write(addr, data);
  SET_FLAG(ZERO, data == 0);
  SET_FLAG(NEGATIVE, data & 0x80);
}

// Increment Y, 1 is added to the Y register
void CPU_6502::INY(uint16_t addr) {
  Y++;
  SET_FLAG(ZERO, X == 0);
  SET_FLAG(NEGATIVE, X & 0x80);
}

// Increment X, 1 is added to the X register
void CPU_6502::INX(uint16_t addr) {
  X++;
  SET_FLAG(ZERO, X == 0);
  SET_FLAG(NEGATIVE, X & 0x80);
}

// Jump, sets the PC to a new value, allowing code to execute from a new
// location JMP BUG: if the new 2 byte PC crosses a page (ends in FF), then the
// CPU fails to increment the page wheen reading the second byte
void CPU_6502::JMP(uint16_t addr) { PC = addr; }

// Pushes the current program counter to the stack, then sets the PC to a new
// value then the code can call a function and return using RTS
// the return address on the stack points 1 byte before the start of the next
// instruction
void CPU_6502::JSR(uint16_t addr) {
  PC--;
  push((PC >> 8));             // push PC hi
  push((PC | ZERO_PAGE_MASK)); // push PC lo
  PC = addr;
}

// Loads a memory value into the accumulator
void CPU_6502::LDA(uint16_t addr) {
  A = read(addr);
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

// Loads a memory value into the X register
void CPU_6502::LDX(uint16_t addr) {
  X = read(addr);
  SET_FLAG(ZERO, X == 0);
  SET_FLAG(NEGATIVE, X & 0x80);
}

// Loads a memory value into the Y register
void CPU_6502::LDY(uint16_t addr) {
  Y = read(addr);
  SET_FLAG(ZERO, Y == 0);
  SET_FLAG(NEGATIVE, Y & 0x80);
}

// Logical shift right, shifts all bits of a memory value or the accumulator one
// position to the right 0 is shifted into bit 7, and 0 is shifted to the carry
// flag equivalent of dividing an unsigned integer by 2 and rounding down, with
// the remainder in the carry flag
void CPU_6502::LSR(uint16_t addr) {

  if (LOOKUP[read(PC - 1)].addr_mode == &CPU_6502::ACCUMULATOR) {
    SET_FLAG(CARRY, A & 0x10);
    A >>= 1;
    SET_FLAG(ZERO, A == 0);
    SET_FLAG(NEGATIVE, false);
  } else {
    uint8_t data = read(addr);
    SET_FLAG(CARRY, data & 0x10);
    data >>= 1;
    write(addr, data);
    SET_FLAG(ZERO, data == 0);
    SET_FLAG(NEGATIVE, false);
  }
}

// No operation, literally just does fucking nothing and wastes space and CPU
// cycles
void CPU_6502::NOP(uint16_t addr) {}

// Bitwise OR, ORs a memory value and the accumulator, bit by bit
void CPU_6502::ORA(uint16_t addr) {
  A |= read(addr);
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

// Push A, stores the value of the accumulator to the current stack position
// then decrements the stack pointer
void CPU_6502::PHA(uint16_t addr) { push(A); }

// Push processor status, pushes a byte representing the status register to the
// stack, then decrements the stack pointer the B flag and the extra bit are
// both pushed as one
void CPU_6502::PHP(uint16_t addr) { push(STATUS_REGISTER | B_FLAG | UNUSED); }

// Pull A, increments the stack pointer then loads that value at that stack
// position into A
void CPU_6502::PLA(uint16_t addr) {
  A = pull();
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

// Pull processor status, increments the stack pointer then loads that byte into
// the 6 flags
void CPU_6502::PLP(uint16_t addr) {
  STATUS_REGISTER = pull();
  SET_FLAG(UNUSED, true);
}

// Rotate left, shifts a memory value or the accumulator to the left
// the carry flag is treated as though it is both above bit 7 and below bit 0
// so bit 7 is shifted to carry, and carry is shifted to bit 0
void CPU_6502::ROL(uint16_t addr) {
  uint8_t carry = GET_FLAG(CARRY) ? 1 : 0;
  if (LOOKUP[read(PC - 1)].addr_mode == &CPU_6502::ACCUMULATOR) {
    uint8_t prev = A;
    A = (A << 1) | carry;
    SET_FLAG(CARRY, prev & 0x80);
    SET_FLAG(ZERO, A == 0);
    SET_FLAG(NEGATIVE, A & 0x80);
  } else {
    uint8_t data = read(addr);
    uint8_t prev = data;
    data = (data << 1) | carry;
    write(addr, data);
    SET_FLAG(CARRY, prev & 0x80);
    SET_FLAG(ZERO, data == 0);
    SET_FLAG(NEGATIVE, data & 0x80);
  }
}

// Rotate right, shifts a memory value or the accumulator to the right
// the carry flag is treated as though it is both above bit 7 and below bit 0
// so bit 7 is shifted to carry, and carry is shifted to bit 0
void CPU_6502::ROR(uint16_t addr) {
  uint8_t carry = GET_FLAG(CARRY) ? 0x80 : 0x00;
  if (LOOKUP[read(PC - 1)].addr_mode == &CPU_6502::ACCUMULATOR) {
    uint8_t prev = A;
    A = (A >> 1) | carry;
    SET_FLAG(CARRY, prev & 0x01);
    SET_FLAG(ZERO, A == 0);
    SET_FLAG(NEGATIVE, A & 0x80);
  } else {
    uint8_t data = read(addr);
    uint8_t prev = data;
    data = (data >> 1) | carry;
    write(addr, data);
    SET_FLAG(CARRY, prev & 0x01);
    SET_FLAG(ZERO, data == 0);
    SET_FLAG(NEGATIVE, data & 0x80);
  }
}

// Return from interrupt, returns from an interrupt handler by pulling the
// status flags from the stack, then pulling the new program counter similar to
// PLP but changes to the interrupt disable flag apply immediately instead of
// being delayed one instruction
void CPU_6502::RTI(uint16_t addr) {
  STATUS_REGISTER = pull();
  SET_FLAG(UNUSED, true);
  uint8_t lo = pull();
  uint8_t hi = pull();
  PC = (hi << 8) | lo;
}

// Return from subroutine, pulls an address from the stack into the program
// counter, then increments the program counter
void CPU_6502::RTS(uint16_t addr) {
  uint8_t lo = pull();
  uint8_t hi = pull();
  PC = (hi << 8) | lo;
  PC++;
}

// Subtract with carry, subtracts a memory value and the NOT of the carry flag
// from the accumulator it does this by adding the bitwise NOT of the memory
// value using ADC carry is cleared when it underflows and set otherwise
// overflow works the same as ADC, except with an intverted memory value
void CPU_6502::SBC(uint16_t addr) {
  const uint16_t value = read(addr) ^ ZERO_PAGE_MASK;
  uint16_t sum = A + value + (GET_FLAG(CARRY) ? 1 : 0);
  SET_FLAG(CARRY, sum & PAGE_MASK);
  SET_FLAG(ZERO, (sum & ZERO_PAGE_MASK) == 0);
  SET_FLAG(OVERFLOW, (sum ^ A) & (sum & value) & 0x80);
  SET_FLAG(NEGATIVE, sum & 0x80);
  A = sum;
}

// Set carry, sets the carry flag
void CPU_6502::SEC(uint16_t addr) { SET_FLAG(CARRY, true); }

// Set decimal, sets the decimal flag
void CPU_6502::SED(uint16_t addr) { SET_FLAG(DECIMAL, true); }

// Set interrupt disable, sets the interrupt disable flag
void CPU_6502::SEI(uint16_t addr) { SET_FLAG(INTERRUPT_DISABLE, true); }

// Store accumulator, stores the accumulator value into memory
void CPU_6502::STA(uint16_t addr) { write(addr, A); }

// Store X, stores the X register value into memory
void CPU_6502::STX(uint16_t addr) { write(addr, X); }

// Store Y, stores the Y register value into memory
void CPU_6502::STY(uint16_t addr) { write(addr, Y); }

// Transfer A to X, copies the accumulator value to the X register
void CPU_6502::TAX(uint16_t addr) {
  X = A;
  SET_FLAG(ZERO, X == 0);
  SET_FLAG(NEGATIVE, X & 0x80);
}

// Transfer A to Y, copies the accumulator value to the Y register
void CPU_6502::TAY(uint16_t addr) {

  Y = A;
  SET_FLAG(ZERO, Y == 0);
  SET_FLAG(NEGATIVE, Y & 0x80);
}

// Transfer stack pointer to X, copies the stack pointer value to the X register
void CPU_6502::TSX(uint16_t addr) {

  X = SP;
  SET_FLAG(ZERO, X == 0);
  SET_FLAG(NEGATIVE, X & 0x80);
}

// Transfer X to the accumulator, copies the X register value to the accumulator
void CPU_6502::TXA(uint16_t addr) {

  A = X;
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

// Transfer X to the stack pointer, copies the X register value to the stack
// pointer
void CPU_6502::TXS(uint16_t addr) { SP = X; }

// Transfer Y to the accumulator, copies the Y register value to the accumulator
void CPU_6502::TYA(uint16_t addr) {
  A = Y;
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

// UNIMPLEMENTED/INVALID

void CPU_6502::SLO(uint16_t addr) {
  uint8_t data = read(addr);
  SET_FLAG(CARRY, data & 0x80);
  data <<= 1;
  write(addr, data);
  A |= data;
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

void CPU_6502::RLA(uint16_t addr) {
  uint8_t data = read(addr);
  uint8_t carry = GET_FLAG(CARRY) ? 1 : 0;
  uint8_t new_carry = (data & 0x80) ? 1 : 0;
  data = (data << 1) | carry;
  write(addr, data);
  SET_FLAG(CARRY, new_carry);
  A &= data;
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

void CPU_6502::SRE(uint16_t addr) {
  uint8_t data = read(addr);
  SET_FLAG(CARRY, data & 0x01);
  data >>= 1;
  write(addr, data);
  A ^= data;
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

void CPU_6502::RRA(uint16_t addr) {
  uint8_t data = read(addr);
  uint8_t carry = GET_FLAG(CARRY) ? 0x80 : 0x00;
  uint8_t new_carry = data & 0x01;
  data = (data >> 1) | carry;
  write(addr, data);
  SET_FLAG(CARRY, new_carry);
  uint16_t sum = A + data + (GET_FLAG(CARRY) ? 1 : 0);
  SET_FLAG(CARRY, sum > ZERO_PAGE_MASK);
  SET_FLAG(ZERO, (sum & ZERO_PAGE_MASK) == 0);
  SET_FLAG(OVERFLOW, (~(A ^ data) & (A ^ sum) & 0x80) != 0);
  SET_FLAG(NEGATIVE, sum & ZERO_PAGE_MASK);
  A = sum;
}

void CPU_6502::SAX(uint16_t addr) {
  uint8_t value = A & X;
  write(addr, value);
}

void CPU_6502::LAX(uint16_t addr) {
  uint8_t data = read(addr);
  A = X = data;
  SET_FLAG(ZERO, data == 0);
  SET_FLAG(NEGATIVE, data & 0x80);
}

void CPU_6502::DCP(uint16_t addr) {
  uint8_t data = read(addr);
  write(addr, data);
  uint16_t difference = A - data;
  SET_FLAG(CARRY, A >= data);
  SET_FLAG(ZERO, (difference & ZERO_PAGE_MASK) == 0);
  SET_FLAG(NEGATIVE, difference & 0x80);
}

void CPU_6502::ISC(uint16_t addr) {
  uint8_t data = read(addr) - 1;
  write(addr, data);
  uint16_t value = data ^ ZERO_PAGE_MASK;
  uint16_t sum = A + value + (GET_FLAG(CARRY) ? 1 : 0);
  SET_FLAG(CARRY, sum & PAGE_MASK);
  SET_FLAG(ZERO, (sum & ZERO_PAGE_MASK) == 0);
  SET_FLAG(OVERFLOW, ((sum ^ A) & (sum ^ value) & 0x80));
  SET_FLAG(NEGATIVE, (sum & 0x80));
  A = sum;
}

void CPU_6502::ANC(uint16_t addr) {
  uint8_t value = read(addr);
  A = A & value;
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(CARRY, A & 0x80);
  SET_FLAG(CARRY, A & 0x80);
}

void CPU_6502::ALR(uint16_t addr) {
  A = A & read(addr);
  SET_FLAG(CARRY, A & 0x01);
  A >>= 1;
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

void CPU_6502::ARR(uint16_t addr) {
  A = A & read(addr);
  uint8_t carry = GET_FLAG(CARRY) ? 0x80 : 0x00;
  uint8_t result = (A >> 1) | carry;
  SET_FLAG(CARRY, result & 0x40);
  SET_FLAG(OVERFLOW, ((result & 0x40) >> 6) ^ ((result & 0x20) >> 5));
  A = result;
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(ZERO, A & 0x80);
}

void CPU_6502::AXS(uint16_t addr) {
  uint8_t value = read(addr);
  uint16_t tmp = (A & X) - value;
  SET_FLAG(CARRY, (tmp & 0x100) == 0);
  X = tmp & ZERO_PAGE_MASK;
  SET_FLAG(ZERO, X == 0);
  SET_FLAG(NEGATIVE, X & 0x80);
}

void CPU_6502::LAS(uint16_t addr) {
  uint8_t M = read(addr) & SP;
  A = X = SP = M;
  SET_FLAG(ZERO, M == 0);
  SET_FLAG(NEGATIVE, M & 0x80);
}

void CPU_6502::SHX(uint16_t addr) {
  uint16_t target = addr;
  uint8_t value = X & ((target >> 8) + 1);
  write(addr, value);
}

void CPU_6502::SHY(uint16_t addr) {
  uint16_t target = addr;
  uint8_t value = Y & ((target >> 8) + 1);
  write(addr, value);
}

void CPU_6502::TAS(uint16_t addr) {
  uint8_t target = read(addr);
  uint8_t value = A & X;
  SP = value;
  write(addr, value & ((target >> 8) + 1));
}

void CPU_6502::XAA(uint16_t addr) {
  uint8_t value = read(addr);
  A = X & value;
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

void CPU_6502::LXA(uint16_t addr) {
  uint8_t value = read(addr);
  A = X = (A | 0x00) & value;
  SET_FLAG(ZERO, A == 0);
  SET_FLAG(NEGATIVE, A & 0x80);
}

/// BUILD LOOKUP TABLE
void CPU_6502::BUILD_LOOKUP() {

  // Lambda for adding instructions to the lookup table
  auto INSERT_INSTRUCTION = [this](uint8_t opcode_byte, const char *mnemonic,
                                   int cycles,
                                   void (CPU_6502::*opcode)(uint16_t),
                                   uint16_t (CPU_6502::*addr_mode)()) {
    auto instruciton_length =
        [&](uint16_t (CPU_6502::*addr_mode)()) -> uint8_t {
      if (addr_mode == &CPU_6502::IMPLICIT ||
          addr_mode == &CPU_6502::ACCUMULATOR) {
        return 1;
      }
      if (addr_mode == &CPU_6502::RELATIVE ||
          addr_mode == &CPU_6502::IMMEDIATE ||
          addr_mode == &CPU_6502::ZERO_PAGE ||
          addr_mode == &CPU_6502::ZERO_PAGE_INDEXED_X ||
          addr_mode == &CPU_6502::ZERO_PAGE_INDEXED_Y ||
          addr_mode == &CPU_6502::INDEXED_INDIRECT ||
          addr_mode == &CPU_6502::INDIRECT_INDEXED) {
        return 2;
      }
      return 3;
    };

    LOOKUP[opcode_byte] = {mnemonic, cycles, instruciton_length(addr_mode),
                           opcode, addr_mode};
  };

  // Fill lookup table with NOP
  for (auto &inst : LOOKUP) {
    inst = {"NOP", 2, 1, &CPU_6502::NOP, &CPU_6502::IMPLICIT};
  };

  // ADC
  INSERT_INSTRUCTION(0x69, "ADC", 2, &CPU_6502::ADC, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0x65, "ADC", 3, &CPU_6502::ADC, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x75, "ADC", 4, &CPU_6502::ADC,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x6D, "ADC", 4, &CPU_6502::ADC, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x7D, "ADC", 4, &CPU_6502::ADC,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x79, "ADC", 4, &CPU_6502::ADC,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0x61, "ADC", 6, &CPU_6502::ADC,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0x71, "ADC", 5, &CPU_6502::ADC,
                     &CPU_6502::INDIRECT_INDEXED);

  // AND
  INSERT_INSTRUCTION(0x29, "AND", 2, &CPU_6502::AND, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0x25, "AND", 3, &CPU_6502::AND, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x35, "AND", 4, &CPU_6502::AND,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x2D, "AND", 4, &CPU_6502::AND, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x3D, "AND", 4, &CPU_6502::AND,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x39, "AND", 4, &CPU_6502::AND,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0x21, "AND", 6, &CPU_6502::AND,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0x31, "AND", 5, &CPU_6502::AND,
                     &CPU_6502::INDIRECT_INDEXED);

  // ASL
  INSERT_INSTRUCTION(0x0A, "ASL", 2, &CPU_6502::ASL, &CPU_6502::ACCUMULATOR);
  INSERT_INSTRUCTION(0x06, "ASL", 5, &CPU_6502::ASL, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x16, "ASL", 6, &CPU_6502::ASL,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x0E, "ASL", 6, &CPU_6502::ASL, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x1E, "ASL", 7, &CPU_6502::ASL,
                     &CPU_6502::ABSOLUTE_INDEXED_X);

  // BRANCH
  INSERT_INSTRUCTION(0x90, "BCC", 2, &CPU_6502::BCC, &CPU_6502::RELATIVE);
  INSERT_INSTRUCTION(0xB0, "BCS", 2, &CPU_6502::BCS, &CPU_6502::RELATIVE);
  INSERT_INSTRUCTION(0xF0, "BEQ", 2, &CPU_6502::BEQ, &CPU_6502::RELATIVE);
  INSERT_INSTRUCTION(0x30, "BMI", 2, &CPU_6502::BMI, &CPU_6502::RELATIVE);
  INSERT_INSTRUCTION(0xD0, "BNE", 2, &CPU_6502::BNE, &CPU_6502::RELATIVE);
  INSERT_INSTRUCTION(0x10, "BPL", 2, &CPU_6502::BPL, &CPU_6502::RELATIVE);
  INSERT_INSTRUCTION(0x50, "BVC", 2, &CPU_6502::BVC, &CPU_6502::RELATIVE);
  INSERT_INSTRUCTION(0x70, "BVS", 2, &CPU_6502::BVS, &CPU_6502::RELATIVE);

  // BIT
  INSERT_INSTRUCTION(0x24, "BIT", 3, &CPU_6502::BIT, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x2C, "BIT", 4, &CPU_6502::BIT, &CPU_6502::ABSOLUTE);

  // BRK
  INSERT_INSTRUCTION(0x00, "BRK", 7, &CPU_6502::BRK, &CPU_6502::IMMEDIATE);

  // CLEAR/SET
  INSERT_INSTRUCTION(0x18, "CLC", 2, &CPU_6502::CLC, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0xD8, "CLD", 2, &CPU_6502::CLD, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x58, "CLI", 2, &CPU_6502::CLI, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0xB8, "CLV", 2, &CPU_6502::CLV, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x38, "SEC", 2, &CPU_6502::SEC, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0xF8, "SED", 2, &CPU_6502::SED, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x78, "SEI", 2, &CPU_6502::SEI, &CPU_6502::IMPLICIT);

  // CMP
  INSERT_INSTRUCTION(0xC9, "CMP", 2, &CPU_6502::CMP, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0xC5, "CMP", 3, &CPU_6502::CMP, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xD5, "CMP", 4, &CPU_6502::CMP,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0xCD, "CMP", 4, &CPU_6502::CMP, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0xDD, "CMP", 4, &CPU_6502::CMP,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0xD9, "CMP", 4, &CPU_6502::CMP,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0xC1, "CMP", 6, &CPU_6502::CMP,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0xD1, "CMP", 5, &CPU_6502::CMP,
                     &CPU_6502::INDIRECT_INDEXED);

  // CPX
  INSERT_INSTRUCTION(0xE0, "CPX", 2, &CPU_6502::CPX, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0xE4, "CPX", 3, &CPU_6502::CPX, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xEC, "CPX", 4, &CPU_6502::CPX, &CPU_6502::ABSOLUTE);

  // CPY
  INSERT_INSTRUCTION(0xC0, "CPY", 2, &CPU_6502::CPY, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0xC4, "CPY", 3, &CPU_6502::CPY, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xCC, "CPY", 4, &CPU_6502::CPY, &CPU_6502::ABSOLUTE);

  // DEC
  INSERT_INSTRUCTION(0xC6, "DEC", 5, &CPU_6502::DEC, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xD6, "DEC", 6, &CPU_6502::DEC,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0xCE, "DEC", 6, &CPU_6502::DEC, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0xDE, "DEC", 7, &CPU_6502::DEC,
                     &CPU_6502::ABSOLUTE_INDEXED_X);

  // DEX/DEY
  INSERT_INSTRUCTION(0xCA, "DEX", 2, &CPU_6502::DEX, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x88, "DEY", 2, &CPU_6502::DEY, &CPU_6502::IMPLICIT);

  // EOR
  INSERT_INSTRUCTION(0x49, "EOR", 2, &CPU_6502::EOR, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0x45, "EOR", 3, &CPU_6502::EOR, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x55, "EOR", 4, &CPU_6502::EOR,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x4D, "EOR", 4, &CPU_6502::EOR, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x5D, "EOR", 4, &CPU_6502::EOR,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x59, "EOR", 4, &CPU_6502::EOR,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0x41, "EOR", 6, &CPU_6502::EOR,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0x51, "EOR", 5, &CPU_6502::EOR,
                     &CPU_6502::INDIRECT_INDEXED);

  // INC
  INSERT_INSTRUCTION(0xE6, "INC", 5, &CPU_6502::INC, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xF6, "INC", 6, &CPU_6502::INC,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0xEE, "INC", 6, &CPU_6502::INC, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0xFF, "INC", 7, &CPU_6502::INC,
                     &CPU_6502::ABSOLUTE_INDEXED_X);

  // INX/INY
  INSERT_INSTRUCTION(0xE8, "INX", 2, &CPU_6502::INX, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0xC8, "INY", 2, &CPU_6502::INX, &CPU_6502::IMPLICIT);

  // JMP/JSR
  INSERT_INSTRUCTION(0x4C, "JMP", 3, &CPU_6502::JMP, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x6C, "JMP", 5, &CPU_6502::JMP, &CPU_6502::INDIRECT);
  INSERT_INSTRUCTION(0x20, "JSR", 6, &CPU_6502::JSR, &CPU_6502::ABSOLUTE);

  // LDA
  INSERT_INSTRUCTION(0xA9, "LDA", 2, &CPU_6502::LDA, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0xA5, "LDA", 3, &CPU_6502::LDA, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xB5, "LDA", 4, &CPU_6502::LDA,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0xAD, "LDA", 4, &CPU_6502::LDA, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0xBD, "LDA", 4, &CPU_6502::LDA,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0xB9, "LDA", 4, &CPU_6502::LDA,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0xA1, "LDA", 6, &CPU_6502::LDA,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0xB1, "LDA", 5, &CPU_6502::LDA,
                     &CPU_6502::INDIRECT_INDEXED);

  // LDX
  INSERT_INSTRUCTION(0xA2, "LDX", 2, &CPU_6502::LDX, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0xA6, "LDX", 3, &CPU_6502::LDX, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xB6, "LDX", 4, &CPU_6502::LDX,
                     &CPU_6502::ZERO_PAGE_INDEXED_Y);
  INSERT_INSTRUCTION(0xAE, "LDX", 4, &CPU_6502::LDX, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0xBE, "LDX", 4, &CPU_6502::LDX,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);

  // LDY
  INSERT_INSTRUCTION(0xA0, "LDY", 2, &CPU_6502::LDY, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0xA4, "LDY", 3, &CPU_6502::LDY, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xB4, "LDY", 4, &CPU_6502::LDY,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0xAC, "LDY", 4, &CPU_6502::LDY, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0xBC, "LDY", 4, &CPU_6502::LDY,
                     &CPU_6502::ABSOLUTE_INDEXED_X);

  // LSR
  INSERT_INSTRUCTION(0x4A, "LSR", 2, &CPU_6502::LSR, &CPU_6502::ACCUMULATOR);
  INSERT_INSTRUCTION(0x46, "LSR", 5, &CPU_6502::LSR, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x56, "LSR", 6, &CPU_6502::LSR,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x4E, "LSR", 6, &CPU_6502::LSR, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x5E, "LSR", 7, &CPU_6502::LSR,
                     &CPU_6502::ABSOLUTE_INDEXED_X);

  // ORA
  INSERT_INSTRUCTION(0x09, "ORA", 2, &CPU_6502::ORA, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0x05, "ORA", 3, &CPU_6502::ORA, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x15, "ORA", 4, &CPU_6502::ORA,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x0D, "ORA", 4, &CPU_6502::ORA, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x1D, "ORA", 4, &CPU_6502::ORA,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x19, "ORA", 4, &CPU_6502::ORA,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0x01, "ORA", 6, &CPU_6502::ORA,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0x11, "ORA", 5, &CPU_6502::ORA,
                     &CPU_6502::INDIRECT_INDEXED);

  // STACK OPERATIONS
  INSERT_INSTRUCTION(0x48, "PHA", 3, &CPU_6502::PHA, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x08, "PHP", 3, &CPU_6502::PHP, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x68, "PLA", 4, &CPU_6502::PLA, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x28, "PLP", 4, &CPU_6502::PLP, &CPU_6502::IMPLICIT);

  // ROL
  INSERT_INSTRUCTION(0x2A, "ROL", 2, &CPU_6502::ROL, &CPU_6502::ACCUMULATOR);
  INSERT_INSTRUCTION(0x26, "ROL", 5, &CPU_6502::ROL, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x36, "ROL", 6, &CPU_6502::ROL,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x2E, "ROL", 6, &CPU_6502::ROL, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x3E, "ROL", 7, &CPU_6502::ROL,
                     &CPU_6502::ABSOLUTE_INDEXED_X);

  // ROR
  INSERT_INSTRUCTION(0x6A, "ROR", 2, &CPU_6502::ROR, &CPU_6502::ACCUMULATOR);
  INSERT_INSTRUCTION(0x66, "ROR", 5, &CPU_6502::ROR, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x76, "ROR", 6, &CPU_6502::ROR,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x6E, "ROR", 6, &CPU_6502::ROR, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x7E, "ROR", 7, &CPU_6502::ROR,
                     &CPU_6502::ABSOLUTE_INDEXED_X);

  // RTI/RTS
  INSERT_INSTRUCTION(0x40, "RTI", 6, &CPU_6502::RTI, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x60, "RTS", 6, &CPU_6502::RTS, &CPU_6502::IMPLICIT);

  // SBC
  INSERT_INSTRUCTION(0xE9, "SBC", 2, &CPU_6502::SBC, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0xE5, "SBC", 3, &CPU_6502::SBC, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xF5, "SBC", 4, &CPU_6502::SBC,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0xED, "SBC", 4, &CPU_6502::SBC, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0xFD, "SBC", 4, &CPU_6502::SBC,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0xF9, "SBC", 4, &CPU_6502::SBC,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0xE1, "SBC", 6, &CPU_6502::SBC,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0xF1, "SBC", 5, &CPU_6502::SBC,
                     &CPU_6502::INDIRECT_INDEXED);

  // STA
  INSERT_INSTRUCTION(0x85, "STA", 3, &CPU_6502::STA, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x95, "STA", 4, &CPU_6502::STA,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x8D, "STA", 4, &CPU_6502::STA, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x9D, "STA", 5, &CPU_6502::STA,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x99, "STA", 5, &CPU_6502::STA,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0x81, "STA", 6, &CPU_6502::STA,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0x91, "STA", 6, &CPU_6502::STA,
                     &CPU_6502::INDIRECT_INDEXED);

  // STX
  INSERT_INSTRUCTION(0x86, "STX", 3, &CPU_6502::STX, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x96, "STX", 4, &CPU_6502::STX,
                     &CPU_6502::ZERO_PAGE_INDEXED_Y);
  INSERT_INSTRUCTION(0x8E, "STX", 4, &CPU_6502::STX, &CPU_6502::ABSOLUTE);

  // STY
  INSERT_INSTRUCTION(0x84, "STY", 3, &CPU_6502::STY, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x94, "STY", 4, &CPU_6502::STY,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x8C, "STY", 4, &CPU_6502::STY, &CPU_6502::ABSOLUTE);

  // TRANSFER OPERATIONS
  INSERT_INSTRUCTION(0xAA, "TAX", 2, &CPU_6502::TAX, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0xA8, "TAY", 2, &CPU_6502::TAY, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0xBA, "TSX", 2, &CPU_6502::TSX, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x8A, "TXA", 2, &CPU_6502::TXA, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x9A, "TXS", 2, &CPU_6502::TXS, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x98, "TYA", 2, &CPU_6502::TYA, &CPU_6502::IMPLICIT);

  // OFFICIAL NOP
  INSERT_INSTRUCTION(0xEA, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMPLICIT);

  // UNOFFICIAL NOP
  INSERT_INSTRUCTION(0x1A, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x3A, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x5A, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x7A, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0xDA, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0xFA, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMPLICIT);
  INSERT_INSTRUCTION(0x80, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0x82, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0x89, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0xC2, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0xE2, "NOP", 2, &CPU_6502::NOP, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0x04, "NOP", 3, &CPU_6502::NOP, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x44, "NOP", 3, &CPU_6502::NOP, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x64, "NOP", 3, &CPU_6502::NOP, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x0C, "NOP", 4, &CPU_6502::NOP, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x14, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x34, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x54, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x74, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0xD4, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0xF4, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x1C, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x3C, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x5C, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x7C, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0xDC, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0xFC, "NOP", 4, &CPU_6502::NOP,
                     &CPU_6502::ABSOLUTE_INDEXED_X);

  // SLO
  INSERT_INSTRUCTION(0x07, "SLO", 5, &CPU_6502::SLO, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x17, "SLO", 6, &CPU_6502::SLO,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x0F, "SLO", 6, &CPU_6502::SLO, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x1F, "SLO", 7, &CPU_6502::SLO,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x1B, "SLO", 7, &CPU_6502::SLO,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0x03, "SLO", 8, &CPU_6502::SLO,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0x13, "SLO", 8, &CPU_6502::SLO,
                     &CPU_6502::INDIRECT_INDEXED);

  // RLA
  INSERT_INSTRUCTION(0x27, "RLA", 5, &CPU_6502::RLA, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x37, "RLA", 6, &CPU_6502::RLA,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x2F, "RLA", 6, &CPU_6502::RLA, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x3F, "RLA", 7, &CPU_6502::RLA,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x3B, "RLA", 7, &CPU_6502::RLA,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0x23, "RLA", 8, &CPU_6502::RLA,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0x33, "RLA", 8, &CPU_6502::RLA,
                     &CPU_6502::INDIRECT_INDEXED);

  // SRE
  INSERT_INSTRUCTION(0x47, "SRE", 5, &CPU_6502::SRE, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x57, "SRE", 6, &CPU_6502::SRE,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x4F, "SRE", 6, &CPU_6502::SRE, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x5F, "SRE", 7, &CPU_6502::SRE,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x5B, "SRE", 7, &CPU_6502::SRE,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0x43, "SRE", 8, &CPU_6502::SRE,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0x53, "SRE", 8, &CPU_6502::SRE,
                     &CPU_6502::INDIRECT_INDEXED);

  // RRA
  INSERT_INSTRUCTION(0x67, "RRA", 5, &CPU_6502::RRA, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x77, "RRA", 6, &CPU_6502::RRA,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0x6F, "RRA", 6, &CPU_6502::RRA, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x7F, "RRA", 7, &CPU_6502::RRA,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x7B, "RRA", 7, &CPU_6502::RRA,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0x63, "RRA", 8, &CPU_6502::RRA,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0x73, "RRA", 8, &CPU_6502::RRA,
                     &CPU_6502::INDIRECT_INDEXED);

  // SAX
  INSERT_INSTRUCTION(0x87, "SAX", 3, &CPU_6502::SAX, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0x97, "SAX", 4, &CPU_6502::SAX,
                     &CPU_6502::ZERO_PAGE_INDEXED_Y);
  INSERT_INSTRUCTION(0x8F, "SAX", 4, &CPU_6502::SAX, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0x83, "SAX", 6, &CPU_6502::SAX,
                     &CPU_6502::INDEXED_INDIRECT);

  // LAX
  INSERT_INSTRUCTION(0xA7, "LAX", 3, &CPU_6502::LAX, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xB7, "LAX", 4, &CPU_6502::LAX,
                     &CPU_6502::ZERO_PAGE_INDEXED_Y);
  INSERT_INSTRUCTION(0xAF, "LAX", 4, &CPU_6502::LAX, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0xBF, "LAX", 4, &CPU_6502::LAX,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0xA3, "LAX", 6, &CPU_6502::LAX,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0xB3, "LAX", 5, &CPU_6502::LAX,
                     &CPU_6502::INDIRECT_INDEXED);

  // DCP
  INSERT_INSTRUCTION(0xC7, "DCP", 5, &CPU_6502::DCP, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xD7, "DCP", 6, &CPU_6502::DCP,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0xCF, "DCP", 6, &CPU_6502::DCP, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0xDF, "DCP", 7, &CPU_6502::DCP,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0xDB, "DCP", 7, &CPU_6502::DCP,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0xC3, "DCP", 8, &CPU_6502::DCP,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0xD3, "DCP", 8, &CPU_6502::DCP,
                     &CPU_6502::INDIRECT_INDEXED);

  // ISC
  INSERT_INSTRUCTION(0xE7, "ISC", 5, &CPU_6502::ISC, &CPU_6502::ZERO_PAGE);
  INSERT_INSTRUCTION(0xF7, "ISC", 6, &CPU_6502::ISC,
                     &CPU_6502::ZERO_PAGE_INDEXED_X);
  INSERT_INSTRUCTION(0xEF, "ISC", 6, &CPU_6502::ISC, &CPU_6502::ABSOLUTE);
  INSERT_INSTRUCTION(0xFF, "ISC", 7, &CPU_6502::ISC,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0xFB, "ISC", 7, &CPU_6502::ISC,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0xE3, "ISC", 8, &CPU_6502::ISC,
                     &CPU_6502::INDEXED_INDIRECT);
  INSERT_INSTRUCTION(0xF3, "ISC", 8, &CPU_6502::ISC,
                     &CPU_6502::INDIRECT_INDEXED);

  // ANC
  INSERT_INSTRUCTION(0x0B, "ANC", 2, &CPU_6502::ANC, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0x2B, "ANC", 2, &CPU_6502::ANC, &CPU_6502::IMMEDIATE);

  // ALR
  INSERT_INSTRUCTION(0x4B, "ALR", 2, &CPU_6502::ALR, &CPU_6502::IMMEDIATE);

  // ARR
  INSERT_INSTRUCTION(0x6B, "ARR", 2, &CPU_6502::ARR, &CPU_6502::IMMEDIATE);

  // AXS
  INSERT_INSTRUCTION(0xCB, "AXS", 2, &CPU_6502::AXS, &CPU_6502::IMMEDIATE);

  // LAS
  INSERT_INSTRUCTION(0xBB, "LAS", 4, &CPU_6502::LAS,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);

  // SHX/SHY/TAS
  INSERT_INSTRUCTION(0x9E, "SHX", 5, &CPU_6502::SHX,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);
  INSERT_INSTRUCTION(0x9C, "SHY", 5, &CPU_6502::SHY,
                     &CPU_6502::ABSOLUTE_INDEXED_X);
  INSERT_INSTRUCTION(0x9B, "TAS", 5, &CPU_6502::TAS,
                     &CPU_6502::ABSOLUTE_INDEXED_Y);

  // XAA/LXA
  INSERT_INSTRUCTION(0x8B, "XAA", 2, &CPU_6502::XAA, &CPU_6502::IMMEDIATE);
  INSERT_INSTRUCTION(0xAB, "LXA", 2, &CPU_6502::LXA, &CPU_6502::IMMEDIATE);
}
