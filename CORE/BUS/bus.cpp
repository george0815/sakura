#include "bus.h"
#include "../LOGGER/logger.h"
#include <array>
#include <cstdint>
// #include <iostream>
#include <sys/types.h>
#include <vector>

using namespace std;

/*The CPU has an addressable range of 64KB, here is the memory map chart
 * curtisey (that is definitely not how you spell it lol) of the NESDEV wiki
 *
 *$0000 - $07FF - 2KB internal CPU RAM
 *
 *$0800 - $1FFF - 2KB -> 8KB, mirrors the original 2KB, I will explain
 * mirroring later on in this file (different from nametable mirroring)
 *
 * $2000 - $2007 - PPU registers, this is accessed through the bus
 *
 * $2008 - $3FFF - Mirrors the PPU registers, basically for every 8 bytes in
 * this range, it will map to a byte in the range of $2000 - $2007 For
 * example: 2008 -> 2000, 2009 -> 2001, etc
 *
 *$4000 - $4017 - NES PPU
 *$4018 - $401F - APU and IO, normally disabled
 *$4020 - $FFFF - unmapped
 *
 * */

uint32_t BUS::fnv1a(const vector<uint8_t> bytes, uint32_t hash) {
  for (uint8_t byte : bytes) {
    hash ^= byte;
    hash *= 16777619u;
  }
  return hash;
}

BUS::~BUS() { delete MAPPER; }

uint8_t BUS::read(uint16_t addr) {

  if (addr == 0x4015) {
    const uint64_t cpu_cycle = CPU ? CPU->TOTAL_CYCLES : 0;
    return APU.read_status(cpu_cycle);
  }

  // If address is within the 8KB addressable range for the CPU RAM
  if (addr < 0x2000) {

    // mirrored every 2KB, ANDing (size - 1) is the basically the same as %
    // size (as long as your dealing with binary numbers) this is because when
    // you decrement a power of 2 (2048, etc) by 1, in binary all the lower
    // bits will be 1, and the bit that was originally 1 is now 0 0x0800 = 2KB
    // = 2048 = 0000 1000 0000 0000 -> 0x07FF = 2047 = 0000 0111 1111 1111
    return CPU_RAM[addr & 0x07FF]; // mirrored every 2KB, ANDing (size - 1) is
                                   // the basically the same as % size (as long
                                   // as your dealing with binary numbers)
  }

  else if (addr >= 0x2000 && addr <= 0x3FFF) {
    const uint16_t reg = 0x2000 + (addr & 0x0007);

    if (reg == 0x2002 || reg == 0x2004 || reg == 0x2007) {
      if (PPU) {
        return PPU->cpu_read(reg);
      }
    }
    return SHADOW[reg];
  }

  if (addr == 0x4016 || addr == 0x4017) {
    const int port = (addr == 0x4016) ? 0 : 1;
    if (CONTROLLER_STROBE) {
      return CONTROLLER_STATE[port] & 0x01;
    }

    const uint8_t value = CONTROLLER_SHIFT[port] & 0x01;
    this->CONTROLLER_SHIFT[port] >>= 1;
    return value | 0x40;
  }

  if (addr < 0x8000) {
    if (addr >= 0x6000 && !PRG_RAM.empty()) {
      return PRG_RAM[addr - 0x6000];
    }
    return SHADOW[addr];
  }

  if (MAPPER) {
    return MAPPER->cpu_read(addr);
  }

  return 0;
}

void BUS::write(uint16_t addr, uint8_t data) {

  // If address falls into the 8KB addressable range for internal CPU CPU_RAM
  if (addr < 0x2000) {
    CPU_RAM[addr & 0x07FF] = data; // here we mirror it every 2KB again
  }

  else if (addr >= 0x2000 && addr <= 0x3FFF) {
    const uint16_t reg = 0x2000 + (addr & 0x0007);

    SHADOW[reg] = data;

    if (PPU) {
      PPU->cpu_write(reg, data);
    }

    return;
  }

  if (addr == 0x4014) {
    SHADOW[addr] = data;
    if (PPU) {
      const uint16_t page_base = data << 8;
      for (uint16_t offset = 0; offset < 0x0100; ++offset) {
        PPU->oam_dma_write(read(page_base + offset));
      }
    }
    return;
  }

  if ((addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015 || addr == 0x4017) {
    SHADOW[addr] = data;
    const uint64_t cpu_cycle = CPU ? CPU->TOTAL_CYCLES : 0;
    APU.write_register(cpu_cycle, addr, data);
    return;
  }

  if (addr == 0x4016) {
    SHADOW[addr] = data;

    bool new_strobe = (data & 0x01) != 0;
    if (CONTROLLER_STROBE && !new_strobe) {
      CONTROLLER_SHIFT = CONTROLLER_STATE;
    }

    CONTROLLER_STROBE = new_strobe;
    return;
  }

  if (addr < 0x8000) {
    if (addr >= 0x6000 && !PRG_RAM.empty()) {
      PRG_RAM[addr - 0x6000] = data;
    } else {
      SHADOW[addr] = data;
    }
    if (MAPPER && addr >= 0x6000) {
      MAPPER->cpu_write(addr, data);
    }
    return;
  }

  if (MAPPER) {
    MAPPER->cpu_write(addr, data);
  }

  return;
}

void BUS::connect_cpu(CPU_6502 &cpu) {
  CPU = &cpu;
  // cout << "BUS <- CPU\n" << endl;
}

void BUS::connect_ppu(PPU_2C02 &ppu) {
  PPU = &ppu;

  if (PPU && MAPPER) {
    PPU->connect_mapper(MAPPER, MIRROR_MODE);
  }
  if (PPU && CPU) {
    PPU->connect_cpu(CPU);
  }
  // cout << "BUS <- PPU\n" << endl;
}

void BUS::SET_CONTROLLER_BUTTON(int port, CONTROLLER_BUTTON button,
                                bool pressed) {

  // cout << "KEY RELEASED: " << button;
  if (port < 0 || port >= CONTROLLER_STATE.size()) {
    return;
  }

  const uint8_t mask = 1u << button;
  if (pressed) {
    CONTROLLER_STATE[port] |= mask;
  } else {
    CONTROLLER_STATE[port] &= ~mask;
  }

  if (CONTROLLER_STROBE) {
    CONTROLLER_SHIFT[port] = CONTROLLER_STATE[port];
  }
}

void BUS::END_AUDIO_FRAME(uint64_t cpu_cycle) { APU.END_FRAME(cpu_cycle); }

vector<int16_t> BUS::TAKE_AUDIO_SAMPLES() { return APU.TAKE_SAMPLES(); }

bool BUS::LOAD_BATTERY_BACKED_SRAM(const vector<uint8_t> &data) {
  if (!HAS_BATTERY_BACKED_SRAM() || data.size() != PRG_RAM.size()) {
    return false;
  }
  // cout << data.size() << "TESTET " << endl;
  PRG_RAM = data;
  return true;
}

void BUS::insert_cartridge(CART &cart) {
  PRG_ROM = cart.PRG;
  CHR_MEM = cart.CHR;
  PRG_RAM.assign(cart.PRG_RAM_SIZE, 0);
  HAS_PRG = !PRG_ROM.empty();
  CHR_IS_RAM = cart.USES_CHR_RAM;
  BATTERY_BACKED = cart.USES_BATTERY_BACKED_SRAM;
  MAPPER_ID = cart.MAPPER_ID;
  MIRROR_MODE = cart.MIRROR_MODE;
  CART_SIGN = fnv1a(CHR_MEM, fnv1a(PRG_ROM));
  if (MAPPER) {
    delete MAPPER;
  }
  switch (MAPPER_ID) {
  case 0:
    MAPPER = new Mapper0(PRG_ROM, CHR_MEM, CHR_IS_RAM, MIRROR_MODE);
    break;
  case 4:
    MAPPER = new Mapper4(PRG_ROM, CHR_MEM, CHR_IS_RAM, MIRROR_MODE);
    break;
  default:
    MAPPER = new Mapper0(PRG_ROM, CHR_MEM, CHR_IS_RAM, MIRROR_MODE);
    break;
  }
  if (PPU) {
    PPU->connect_mapper(MAPPER, MIRROR_MODE);
  }
}

void BUS::save_state(StateWriter &writer) const {
  writer.plain_data(2);
  writer.plain_data(MAPPER_ID);
  writer.plain_data(CART_SIGN);
  writer.array(CPU_RAM);
  writer.array(SHADOW);
  writer.bytes(CHR_MEM);
  writer.bytes(PRG_RAM);
  writer.plain_data(HAS_PRG);
  writer.plain_data(CHR_IS_RAM);
  writer.plain_data(BATTERY_BACKED);
  writer.plain_data(CONTROLLER_STROBE);
  writer.array(CONTROLLER_STATE);
  writer.array(CONTROLLER_SHIFT);
  writer.plain_data(MIRROR_MODE);
  APU.save_state(writer);
  if (MAPPER) {
    MAPPER->save_state(writer);
  }
}

bool BUS::load_state(StateReader &reader) {
  uint32_t version = 0;
  uint16_t saved_mapper_id = 0;
  uint32_t saved_signature = 0;
  vector<uint8_t> saved_chr_mem;
  vector<uint8_t> saved_prg_ram;
  bool saved_has_prg = false;
  bool saved_chr_is_ram = false;
  bool saved_battery_backed = false;

  if (!reader.plain_data(version) || version != 2 ||
      !reader.plain_data(saved_mapper_id) || saved_mapper_id != MAPPER_ID ||
      !reader.plain_data(saved_signature) || saved_signature != CART_SIGN ||
      !reader.array(CPU_RAM) || !reader.array(SHADOW) ||
      !reader.bytes(saved_chr_mem) || !reader.bytes(saved_prg_ram) ||
      !reader.plain_data(saved_has_prg) ||
      !reader.plain_data(saved_chr_is_ram) ||
      !reader.plain_data(saved_battery_backed) ||
      !reader.plain_data(CONTROLLER_STROBE) ||
      !reader.array(CONTROLLER_STATE) || !reader.array(CONTROLLER_SHIFT) ||
      !reader.plain_data(MIRROR_MODE)

  ) {
    // cout << "BUS LOAD STATE FAILED" << endl;
    return false;
  }

  if (saved_has_prg != HAS_PRG || saved_chr_is_ram != CHR_IS_RAM ||
      saved_battery_backed != BATTERY_BACKED ||
      saved_chr_mem.size() != CHR_MEM.size() ||
      saved_prg_ram.size() != PRG_RAM.size()) {
    // cout << " SOME OTHER BUS THING LOAD STATE FAILED" << endl;
    return false;
  }

  CHR_MEM = saved_chr_mem;
  PRG_RAM = saved_prg_ram;
  if (!APU.load_state(reader)) {
    // cout << "APU LOAD STATE FAILED" << endl;
    return false;
  }

  if (MAPPER && !MAPPER->load_state(reader)) {
    // cout << "MAPPER LOAD STATE FAILED" << endl;
    return false;
  }

  return reader.ok();
}
