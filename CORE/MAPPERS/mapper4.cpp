#include "../CPU_6502/core.h"
#include "mapper.h"
#include <cstddef>
#include <cstdint>
#include <vector>

using namespace std;

Mapper4::Mapper4(const vector<uint8_t> &prg, vector<uint8_t> &chr, bool chr_ram,
                 MIRRORING mirror_mode)
    : PRG_ROM(prg), CHR_MEM(chr), CHR_IS_RAM(chr_ram),
      mirror_mode(mirror_mode) {}

size_t Mapper4::PRG_BANK_COUNT() const {
  return PRG_ROM.empty() ? 1 : (PRG_ROM.size() / PRG_BANK_SIZE);
}

size_t Mapper4::CHR_BANK_COUNT() const {
  return CHR_MEM.empty() ? 1 : (CHR_MEM.size() / CHR_BANK_SIZE);
}

size_t Mapper4::PRG_BANK_INDEX(int slot) const {

  const size_t bank_count = PRG_BANK_COUNT();
  const size_t last_bank = bank_count - 1;
  const size_t second_last_bank = (bank_count >= 2) ? bank_count - 2 : 0;

  switch (slot) {

  case 0:
    return PRG_MODE ? second_last_bank : (BANK_REGS[6] % bank_count);
  case 1:
    return BANK_REGS[7] % bank_count;
  case 2:
    return PRG_MODE ? (BANK_REGS[6] % bank_count) : second_last_bank;
  default:
    return last_bank;
  }
}

size_t Mapper4::CHR_BANK_INDEX(uint16_t addr) const {
  const size_t bank_count = CHR_BANK_COUNT();
  const size_t CHR_SLOT = addr / CHR_BANK_SIZE;

  if (!CHR_INVERSION) {
    switch (CHR_SLOT) {
    case 0:
    case 1:
      return ((BANK_REGS[0] & 0xFE) + (CHR_SLOT & 0x01)) % bank_count;
    case 2:
    case 3:
      return ((BANK_REGS[1] & 0xFE) + (CHR_SLOT & 0x01)) % bank_count;
    case 4:
      return BANK_REGS[2] % bank_count;
    case 5:
      return BANK_REGS[3] % bank_count;
    case 6:
      return BANK_REGS[4] % bank_count;
    default:
      return BANK_REGS[5] % bank_count;
    }
  }

  switch (CHR_SLOT) {

  case 0:
    return BANK_REGS[2] % bank_count;
  case 1:
    return BANK_REGS[3] % bank_count;
  case 2:
    return BANK_REGS[4] % bank_count;
  case 3:
    return BANK_REGS[5] % bank_count;
  case 4:
  case 5:
    return ((BANK_REGS[0] & 0xFE) + (CHR_SLOT & 0x01)) % bank_count;
  default:
    return ((BANK_REGS[1] & 0xFE) + ((CHR_SLOT - 6) & 0x01)) % bank_count;
  }
}

uint8_t Mapper4::cpu_read(uint16_t addr) {
  if (addr < PRG_BASE || PRG_ROM.empty()) {
    return 0;
  }

  const size_t slot = (addr - PRG_BASE) / PRG_BANK_SIZE;
  const size_t offset = (addr - PRG_BASE) % PRG_BANK_SIZE;

  return PRG_ROM[PRG_BANK_INDEX(slot) * PRG_BANK_SIZE + offset];
}

void Mapper4::cpu_write(uint16_t addr, uint8_t data) {
  if (addr < PRG_BASE) {
    return;
  }

  if (addr <= 0x9FFF) {
    if ((addr & 0x01) == 0) {
      BANK_SELECT = (data & 0x07);
      PRG_MODE = (data & 0x40) != 0;
      CHR_INVERSION = (data & 0x80) != 0;
    } else {
      BANK_REGS[BANK_SELECT] = data;
    }
    return;
  }

  if (addr <= 0xBFFF) {
    if ((addr & 0x01) == 0) {
      if (mirror_mode != MIRRORING::FOUR_SCREEN) {
        mirror_mode =
            (data & 0x01) ? MIRRORING::HORIZONTAL : MIRRORING::VERTICAL;
      }
    }
    return;
  }

  if (addr <= 0xDFFF) {
    if ((addr & 0x01) == 0) {
      IRQ_LATCH = data;
    } else {
      IRQ_RELOAD = true;
    }
    return;
  }

  if ((addr & 0x01) == 0) {
    IRQ_ENABLED = false;
  } else {
    IRQ_ENABLED = true;
  }
}

uint8_t Mapper4::ppu_read(uint16_t addr) {

  if (addr >= PATTERN_TABLE_END || CHR_MEM.empty()) {
    return 0;
  }

  const size_t bank = CHR_BANK_INDEX(addr);
  const size_t offset = addr % CHR_BANK_SIZE;
  return CHR_MEM[bank * CHR_BANK_SIZE + offset];
}

void Mapper4::ppu_write(uint16_t addr, uint8_t data) {
  if (!CHR_IS_RAM || addr >= PATTERN_TABLE_END || CHR_MEM.empty()) {
    return;
  }

  const size_t bank = CHR_BANK_INDEX(addr);
  const size_t offset = addr % CHR_BANK_SIZE;
  CHR_MEM[bank * CHR_BANK_SIZE + offset] = data;
}

void Mapper4::CLOCK_IRQ_COUNTER(CPU_6502 *cpu) {
  if (IRQ_COUNTER == 0 || IRQ_RELOAD) {
    IRQ_COUNTER = IRQ_LATCH;
    IRQ_RELOAD = false;
  } else {
    IRQ_COUNTER--;
  }
  if (IRQ_COUNTER == 0 && IRQ_ENABLED && cpu) {
    cpu->IRQ_HANDLER();
  }
}

void Mapper4::on_ppu_scanline(CPU_6502 *cpu) { CLOCK_IRQ_COUNTER(cpu); }
