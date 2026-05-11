#pragma once

#include "core.h"
#include <cstdint>
#include <iostream>
#include <string>

using namespace std;

/*SOME EXPLANATIONS ABOUT ADDRESSING MODES
 *
 *Addressing modes basically tell the instruction where to find the operand/data
 *
 *
 * A "page" is (in this context) a 256 byte block of memory, so the zero page is
 * from 0x0000 to 0x00FF page one would be from 0x0100 to 0x01FF, page two would
 * be 0x0200 - 0x02FF, and so on
 *
 *
 * NES has 13 addressing modes, comprising of 6 indexed (basically, CPU takes
 * the base address and adds a register) and 7 other modes An addressing mode
 * basically tells the CPU where the data to conduct a given operation is, ie:
 * returns an address So for example for opcodes that use implicit addressing,
 * the location of the operand is implied, for immediate addressing, the
 * operand itself is used rather than an address
 *
 *
 * */

// Anonymous namespace for variables that are only accessed in this file
namespace {

// CONSTANTS
constexpr uint16_t PAGE_MASK =
    0xFF00; // Used for detected whether a page has been crossed
constexpr uint16_t ZERO_PAGE_MASK =
    0x00FF; // Used for mirroring address every 256 byte/ for zero page
} // namespace
// OTHER ADDRESSING MODES

// In implicit addressing, the operand is already implied in the opcode, so just
// returning 0 is fine
uint16_t CPU_6502::IMPLICIT() { return 0; }

// In accumulator addressing, the operand is the accumulator (yea, no shit
// right? lol), so again, just returning 0 is fine
uint16_t CPU_6502::ACCUMULATOR() { return 0; }

// For immediate addressing, the operand is the byte after the opcode, so we
// just increment the program counter by one
// 1. read(PC++) is called in the CPU clock function, getting the byte at the
// address that is the value of the program counter
// 2. Respective addressing mode is called, IMMEDIATE returns the program
// counter, then increments
// 3. Opcode then uses that as the operand address for the operation (LDA, STA,
// etc)
uint16_t CPU_6502::IMMEDIATE() { return PC++; }

// For zero page addressing the operand address is the data at the address that
// is the value of the program counter, the data that is returned from read is
// uint8_t, so the hi byte is implicitly 0x00 when it gets return as uint16_t
uint16_t CPU_6502::ZERO_PAGE() { return static_cast<uint16_t>(read(PC++)); }

// For absolute addressing, and address is constructed by reading the next 2
// bytes using the program counter I then contruct then aboslute address by
// getting the lo byte and hi byte, then combining them
uint16_t CPU_6502::ABSOLUTE() {
  uint8_t lo = read(PC++);
  uint8_t hi = read(PC++);

  uint16_t addr =
      (hi << 8) | lo; // this effective combines the hi and lo into one address,
                      // shifts the hi 8 bits to the left, then ORs with the lo

  return addr;
}

// For relative addressing, gets a signed byte then adds/subtracts that value
// from the PC, now program excecution proceeds from the new PC value C++ will
// automatically convert a uint8_t to a signed int8_t using two's complement
// This is really only used for branch instructions
uint16_t CPU_6502::RELATIVE() {

  int8_t offset = static_cast<uint8_t>(read(PC++));
  // cout << hex << (int)offset << endl;
  return (PC + offset);
}

// For indirect addressing, the hi byte is read in first, then the lo byte
// Then a new address is constructed
// Using that new address, I read in another hi byte then low byte to construct
// another address
// The program counter is then set to the new address then excecution procceeds
// from there

uint16_t CPU_6502::INDIRECT() {

  uint8_t lo = read(PC++);
  uint8_t hi = read(PC++);

  uint16_t addr = (hi << 8) | lo; // combine to make the address

  // Then use the new address to construct ANOTHER address, now by getting the
  // lo first then the hi (man this is confusing sometimes lol)
  uint8_t new_lo = read(addr);

  // JMP BUG, if page boundary is crossed (for example: $70FF + 1), then the CPU
  // fails to carry the lo order byte so instead of $7100, ($70FF + 1) would
  // become $7000 it basically wraps the lo byte back to 0, and doesn't chnage
  // the hi byte

  uint8_t new_hi = 0x00;
  uint16_t new_hi_location;
  // Detect page crossing
  if (((addr + 1) & PAGE_MASK) !=
      (addr & PAGE_MASK)) { // If high byte was changed, page was crossed, this
                            // is because the lo byte represents up to 256
                            // values with a max value being 255
    // so if you need to represent say, 256, you would have to use the 9th bit,
    // thus changing the hi byte and crossing the page this pattern repeats for
    // every 256 bytes

    new_hi_location = addr & PAGE_MASK; // wrap lo byte back to 0
    new_hi = read(new_hi_location);
  }
  // proceed normally
  else {
    new_hi = read(addr + 1);
  }

  uint16_t new_addr = (new_hi << 8) | new_lo; // combine final address

  return new_addr; // add to program counter then return
}

// INDEXED ADDRESSING MODES

// For zero page indexed x addressing, we get the next byte, then add the x
// register, finally we get the zero page by ANDing it with a 2 byte mask
uint16_t CPU_6502::ZERO_PAGE_INDEXED_X() {

  // This can be done in one line but I want make this readable so I'm gonna do
  // it step by step
  uint8_t data = read(PC++);    // get data
  uint16_t combined = data + X; // add the X register
  uint16_t addr =
      combined & ZERO_PAGE_MASK; // get the zero page address, the mask is
                                 // 0x00FF so it effectivle just gets the low
                                 // byte also mirrors every 256 bytes
  return addr;
}

// For zero page indexed y addressing, we get the next byte, then add the y
// register, finally we get the zero page by ANDing it with a 2 byte mask
uint16_t CPU_6502::ZERO_PAGE_INDEXED_Y() {

  // This can be done in one line but I want make this readable so I'm gonna do
  // it step by step
  uint8_t data = read(PC++);    // get data
  uint16_t combined = data + Y; // add the y register
  uint16_t addr =
      combined & ZERO_PAGE_MASK; // get the zero page address, the mask is
                                 // 0x00FF so it effectivle just gets the low
                                 // byte also mirrors every 256 bytes
  return addr;
}

// For aboslute indexed y addressing, it basically is the same as absolute,
// except the y register is added to it
uint16_t CPU_6502::ABSOLUTE_INDEXED_Y() {

  uint8_t lo = read(PC++);
  uint8_t hi = read(PC++);

  uint16_t addr =
      (hi << 8) | lo; // this effective combines the hi and lo into one address,
                      // shifts the hi 8 bits to the left, then ORs with the lo
  uint16_t new_addr = addr + Y; // add the y register

  PAGE_CROSSED = (addr & PAGE_MASK) !=
                 (new_addr & PAGE_MASK); // if 256 byte interval is crossed

  return new_addr;
}

// For aboslute indexed x addressing, it basically is the same as absolute,
// except the y register is added to it
uint16_t CPU_6502::ABSOLUTE_INDEXED_X() {

  uint8_t lo = read(PC++);
  uint8_t hi = read(PC++);

  uint16_t addr =
      (hi << 8) | lo; // this effective combines the hi and lo into one address,
                      // shifts the hi 8 bits to the left, then ORs with the lo
  uint16_t new_addr = addr + X; // add the x register

  PAGE_CROSSED = (addr & PAGE_MASK) !=
                 (new_addr & PAGE_MASK); // if 256 byte interval is crossed

  return new_addr;
}

// Alright so these two are a fucking doozy, I took a look as nesdev.org and
// this is the formulas for these last two addressing modes (d,x) 	Indexed
// indirect 	val = PEEK(PEEK((arg + X) % 256) + PEEK((arg + X + 1) % 256) *
// 256) (d),y 	Indirect indexed 	val = PEEK(PEEK(arg) + PEEK((arg + 1) %
// 256) * 256 + Y)

uint16_t CPU_6502::INDEXED_INDIRECT() {

  // Alright so lets break this down
  uint8_t first_value = read(PC++); // This is the "arg"

  // this is (arg + X) % 256, ANDing the zero page mask is effectivly % 256
  uint8_t new_lo = read((first_value + X) & ZERO_PAGE_MASK);
  // and this is (arg + X + 1) % 256
  uint8_t new_hi = read((first_value + X + 1) & ZERO_PAGE_MASK);

  // combining the bytes by shifting the hi 8 bits effectivly multiplies it by
  // 256
  uint16_t new_addr = (new_hi << 8) | new_lo;

  return new_addr;
}
//(d),y 	Indirect indexed 	val = PEEK(PEEK(arg) + PEEK((arg + 1) %
// 256) * 256 + Y)
// So for this one, I get the zero page address from the value pointed to by the
// PC, then use that as the location for the new address I then read the lo byte
// then the hi byte, then add the Y register to it, the ZERO_PAGE_MASKs are just
// there for explicitness since read returns  a uint8_t the new lo is already
// effectivly zero page'd
uint16_t CPU_6502::INDIRECT_INDEXED() {

  // arg
  uint8_t first_value = read(PC++);
  // PEEK(arg)
  uint8_t new_lo = read(first_value & ZERO_PAGE_MASK);
  // PEEK((arg + 1) % 256)
  uint8_t new_hi = read((first_value + 1) & ZERO_PAGE_MASK);

  // * 256
  uint16_t new_addr = (new_hi << 8) | new_lo;

  PAGE_CROSSED = ((new_addr + Y) & PAGE_MASK) !=
                 (new_addr & PAGE_MASK); // if 256 byte interval is crossed

  return new_addr + Y;
}
