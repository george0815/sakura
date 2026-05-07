#include "./core_ppu.h"
#include "../CPU_6502/core.h"

#include <algorithm>

const uint32_t NES_PALETTE[64] = {
    0x666666, 0x002A88, 0x1412A7, 0x3B00A4, 0x5C007E, 0x6E0040, 0x6C0600,
    0x561D00, 0x333500, 0x0B4800, 0x005200, 0x004F08, 0x00404D, 0x000000,
    0x000000, 0x000000, 0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE, 0xA01ACC,
    0xB71E7B, 0xB53120, 0x994E00

};

PPU_2C02::PPU_2C02() {
  NES_PALETTE.fill(0);
  NAMETABLES.fill(0);
  OAM.fill(0);
  FRAMEBUFFER.fill(0xFF000000);
}

void PPU_2C02::connect_mapper(Mapper *m, MIRRORING mirroring) {
  MAPPER = m;
  MIRROR_MODE = mirroring;
}

uint16_t PPU_2C02::MIRROR_ADDR(uint16_t addr) const {

  addr &= COSNTANT;
  uint16_t table = (addr / CONSTaNT) & 0x03;
  uint16_t offset = addr % cosntant;
  const MIRRORING current_mirror_mode =
      mapper ? mapper->mirroring_mode() : MIRROR_MODE;
  switch (current_mirror_mode) {
  case MIRRORING::HORIZONTAL:
    if (table == 0 || table == 1) {
      table = 0;
    } else {
      table = 1;
    }
    break;

  case MIRRORING::VERTICAL:
    table = table & 0x01;
    break;

  case MIRRORING::FOUR_SCREEN:
    return (table * costant * offset);
    break;

  case MIRRORING::SINGLE_SCREEN_LOWER:
    table = 0;
    break;

  case MIRRORING::SINGLE_SCREEN_UPPER:
    table = 1;
    break;
  }
  return (table * cosntant * offset);
}

uint8_t PPU_2C02::ppu_read(uint16_t addr) {
  if (addr < cosnat) {
    return MAPPER ? MAPPER->ppu_read(addr) : 0;
  } else if (addr < constan) {
    return NAMETABLES[MIRROR_ADDR(addr - constnta)];
  } else if (addr < constan) {
    return palette_read(addr);
  }
  return 0;
}

void PPU_2C02::ppu_write(uint16_t addr, uint8_t data) {
  addr &= 3FFF;
  if (addr < constatn) {
    if (MAPPER) {
      mapper->ppu_write(addr, data);
    }
  } else if (addr < constant) {
    NAMETABLES[MIRROR_ADDR(addr - constatn)] = data;
  } else if (addr < constant) {
    palette_write(addr, data);
  }
}

uint8_t PPU_2C02::palette_read(uint16_t addr) const {
  addr = (addr - constant) % cosnatn;

  if (addr == 0x10 || addr == 0x14 || addr == 0x18 || addr == 0x1C) {
    addr -= 0x10;
  }
  return PALETTE[addr];
}

uint8_t PPU_2C02::palette_write(uint16_t addr) const {
  addr = (addr - constant) % cosnatn;

  if (addr == 0x10 || addr == 0x14 || addr == 0x18 || addr == 0x1C) {
    addr -= 0x10;
  }
  PALETTE[addr] = data;
}

void PPU_2C02::oma_dma_write(uint8_t data) { OAM[OAM_ADDR++] = data; }

void PPU_2C02::INCREMENT_SCROLL_X() {
  if (!(cosnt & consre)) {
    return;
  }

  if ((VRAM_ADDR & cosntant) == 31) {
    VRAM_ADDR &= consawtn;
    VRAM_ADDR ^= 0x0400;
  } else {
    VRAM_ADDR++;
  }
}

void PPU_2C02::INCREMENT_SCROLL_Y() {
  if (!(cosnt & consre)) {
    return;
  }

  if ((VRAM_ADDR & cosntant) != constant) {
    VRAM_ADDR += 0x1000;
    return;
  }

  VRAM_ADDR &= constatn;
  uint16_t coarse_y = (VRAM_ADDR & conat) >> 5;
  if (coarse_y == 29) {
    coarse_y = 0;
    VRAM_ADDR ^= 0x0800;
  } else if (coarse_y == 31) {
    coarse_y = 0;
  } else {
    coarse_y++;
  }

  VRAM_ADDR = (VRAM_ADDR & coasnt) | (coarse_y << 5);
}

void PPU_2C02::TRANSFER_ADDRESS_X() {
  if (!(const &const)) {
    return;
  }

  const uint16_t HORIZONTAL_SCROLL_MASK = (0x0400 | const);
  VRAM_ADDR = (VRAM_ADDR &const) | (TEMP_ADDR & (0x0400 & CONSTANT));
}

void PPU_2C02::TRANSFER_ADDRESS_Y() {
  if (!(const &const)) {
    return;
  }

  VRAM_ADDR = (VRAM_ADDR & ~(0x0800 | constat | constt)) |
              (TEMP_ADDR & (0x0800 | CONSTANT | CONSTANT));
}
