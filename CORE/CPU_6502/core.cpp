#include "core.h"
#include <cstdint>
#include <iostream>
#include <string>

using namespace std;

// Helper function for branching, takes in a condition and a target
// checks for page crossing, then sets new PC
// Every time branch is called it costs a cycle, with an addional extra cycle if
// a page is crossed
void CPU_6502::BRANCH(uint16_t target, bool condition) {

  if (condition) {

    CYCLES++; // increment cycles

    if ((target & PAGE_MASK) != (PC & PAGE_MASK)) { // check for page crossing
      CYCLES++;                                     // increment cycle again
      PC = target;                                  // set PC to target
    } else {
      PC = target;
    }
  }
}

// Reset, basically just resets the CPU to a known state
// The status register is cleared except for the unused bit
// A new program counter is then read from the reset vector, which is 0xFFFC
void CPU_6502::RESET_HANDLER() {

  // clear status register except unused bit
  STATUS_REGISTER = 0x00 | UNUSED | INTERRUPT_DISABLE;

  // clear other registers
  A = 0x00;
  X = 0x00;
  Y = 0x00;

  // Reset stack pointer, technically the stack pointer is initally xFF
  // But since the PC and status register are pushed at start up, it is
  // effectively 0xFD
  SP = 0xFD;

  // construct new PC from vecor
  uint8_t lo = read(RESET_VECTOR);
  uint8_t hi = read(RESET_VECTOR + 1);
  PC = (hi << 8) | lo;

  // Set cycles
  CPU_6502::CYCLES = 8;
}

// Interrupt request, its similar to a reset in the sense that a new PC is
// loaded from a vector But can only happen if the interrupt disable flag is set
// to 0 The current instruction finishes, PC and status register are stored on
// the stack, then a new PC is loaded from the IRQ vector
void CPU_6502::IRQ_HANDLER() {

  if (!CPU_6502::GET_FLAG(CPU_6502::STATUS::INTERRUPT_DISABLE)) {

    // Push program counter to the stack
    push((PC >> 8) & 0x00FF); // push hi byte first since then the lo byte will
                              // be pulled first
    push(PC & 0x00FF);        // push lo byte

    // Set flags and push status register
    SET_FLAG(B_FLAG, 0);
    SET_FLAG(UNUSED, 1);
    SET_FLAG(INTERRUPT_DISABLE, 1);
    push(STATUS_REGISTER);

    // construct new PC from vecor
    uint8_t lo = read(IRQ_VECTOR);
    uint8_t hi = read(IRQ_VECTOR + 1);
    PC = (hi << 8) | lo;

    // Set cycles
    CYCLES = 7;
  }
}

// NMI, basically the same as an IRQ but it cannot be disabled
// A new program counter is then read from the NMI vector, which is 0xFFFA
void CPU_6502::NMI_HANDLER() {

  // Push program counter to the stack
  push(
      (PC >> 8) &
      0x00FF); // push hi byte first since then the lo byte will be pulled first
  push(PC & 0x00FF); // push lo byte

  // Set flags and push status register
  SET_FLAG(B_FLAG, 0);
  SET_FLAG(UNUSED, 1);
  SET_FLAG(INTERRUPT_DISABLE, 1);
  push(STATUS_REGISTER);

  // construct new PC from vecor
  uint8_t lo = read(NMI_VECTOR);
  uint8_t hi = read(NMI_VECTOR + 1);
  PC = (hi << 8) | lo;

  // Set cycles
  CYCLES = 8;
}

// Read a value from memory, this basically just calls the bus's read fucntion,
// which determines what region the CPU is access (stack, 2kb internal ram, etc)
uint8_t CPU_6502::read(uint16_t addr) { return B->read(addr); }

// Write a value to memory, again, basically just calls the buses read function
void CPU_6502::write(uint16_t addr, uint8_t data) { B->write(addr, data); }

// Push a value onto the stack on page one
// I construct the address by just adding the stack pointer to the end of the
// first page
void CPU_6502::push(uint8_t data) {

  write(0x0100 + SP, data);
  SP--;
}

CPU_6502::CPU_6502() { BUILD_LOOKUP(); }

void CPU_6502::step() {

  // No cycles left, time to execute another instruction
  if (CYCLES == 0) {

    // DEBUG LOGS
    cout << uppercase << hex << PC << " ";

    // fetch instruction using PC
    uint8_t opcode_byte = read(PC++);
    OPCODE &op = LOOKUP[opcode_byte];

    cout << hex << uppercase << (int)opcode_byte << " " << op.mnemonic << " $";
    // Reset page crossed
    PAGE_CROSSED = false;

    // Execute addressing mode function
    uint16_t addr = (this->*op.addr_mode)();
    cout << hex << uppercase << addr << "          ";
    // Set cycles
    CYCLES = op.cycles;

    cout << "A:" << hex << (int)A << " ";
    cout << "X:" << hex << (int)X << " ";
    cout << "Y:" << hex << (int)Y << " ";
    cout << "P:" << hex << (int)STATUS_REGISTER << " ";
    cout << "SP:" << hex << (int)SP << endl;

    // Execute instruction
    (this->*op.opcode)(addr);

    if (PAGE_CROSSED) {
      CYCLES++;
    }
  }

  CYCLES--;

  if (B && B->PPU) {
    for (int i = 0; i < 3; ++i) {
      B->PPU->step();
    }
  }
}

// Pull a value from the stack on page one
// I construct the address by just adding the stack pointer to the end of the
// first page
uint8_t CPU_6502::pull() {

  SP++;
  uint8_t data = read(0x0100 + SP);
  return data;
}

// Helper function for setting flags
void CPU_6502::SET_FLAG(STATUS value, bool condition) {

  // cout << " " << to_string(value) << " :" << to_string(condition) << endl;

  // cout << "P:" << (int)STATUS_REGISTER << " " << endl;
  STATUS_REGISTER =
      condition ? (STATUS_REGISTER | (value)) : (STATUS_REGISTER & (~value));
  //  cout << "P:" << (int)STATUS_REGISTER << " " << endl;
}

void CPU_6502::connect_bus(BUS *b) {
  B = b;
  cout << "CPU <- BUS";
}

// Helper function for getting flags
bool CPU_6502::GET_FLAG(STATUS value) {

  return (STATUS_REGISTER & value) !=
         0x00; // If flag is clear, then this evaluates to false
               //
}
