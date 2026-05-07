#include "./core_ppu.h"
#include "../CPU_6502/core.h"

#include <algorithm>
#include <cstdint>
#include <pthread.h>

const uint32_t NES_PALETTE[64] = {
    0x666666, 0x002A88, 0x1412A7, 0x3B00A4, 0x5C007E, 0x6E0040, 0x6C0600,
    0x561D00, 0x333500, 0x0B4800, 0x005200, 0x004F08, 0x00404D, 0x000000,
    0x000000, 0x000000, 0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE, 0xA01ACC,
    0xB71E7B, 0xB53120, 0x994E00

};

PPU_2C02::PPU_2C02() {
  PALETTE.fill(0);
  NAMETABLES.fill(0);
  OAM.fill(0);
  FRAMEBUFFER.fill(0xFF000000);
}

void PPU_2C02::connect_mapper(Mapper *m, MIRRORING mirroring) {
  MAPPER = m;
  MIRROR_MODE = mirroring;
}

uint16_t PPU_2C02::MIRROR_ADDR(uint16_t addr) const {

  addr &= NAMETABLE_REGION_MASK;
  uint16_t table = (addr / NAMETABLE_SIZE) & 0x03;
  uint16_t offset = addr % NAMETABLE_SIZE;
  const MIRRORING current_mirror_mode =
      MAPPER ? MAPPER->mirroring_mode() : MIRROR_MODE;
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
    return (table * NAMETABLE_SIZE * offset);
    break;

  case MIRRORING::SINGLE_SCREEN_LOWER:
    table = 0;
    break;

  case MIRRORING::SINGLE_SCREEN_UPPER:
    table = 1;
    break;
  }
  return (table * NAMETABLE_SIZE * offset);
}

uint8_t PPU_2C02::ppu_read(uint16_t addr) {
  if (addr < PATTERN_TABLE_END) {
    return MAPPER ? MAPPER->ppu_read(addr) : 0;
  } else if (addr < PALETTE_BASE) {
    return NAMETABLES[MIRROR_ADDR(addr - NAMETABLE_BASE)];
  } else if (addr < PPU_ADDRESS_RANGE_END) {
    return palette_read(addr);
  }
  return 0;
}

void PPU_2C02::ppu_write(uint16_t addr, uint8_t data) {
  addr &= 0x3FFF;
  if (addr < PATTERN_TABLE_END) {
    if (MAPPER) {
      MAPPER->ppu_write(addr, data);
    }
  } else if (addr < PALETTE_BASE) {
    NAMETABLES[MIRROR_ADDR(addr - NAMETABLE_BASE)] = data;
  } else if (addr < PPU_ADDRESS_RANGE_END) {
    palette_write(addr, data);
  }
}

uint8_t PPU_2C02::palette_read(uint16_t addr) const {
  addr = (addr - PALETTE_BASE) % PALETTE_MIRROR_SIZE;

  if (addr == 0x10 || addr == 0x14 || addr == 0x18 || addr == 0x1C) {
    addr -= 0x10;
  }
  return PALETTE[addr];
}

void PPU_2C02::palette_write(uint16_t addr, uint8_t data) {
  addr = (addr - PALETTE_BASE) % PALETTE_MIRROR_SIZE;

  if (addr == 0x10 || addr == 0x14 || addr == 0x18 || addr == 0x1C) {
    addr -= 0x10;
  }
  PALETTE[addr] = data;
}

void PPU_2C02::oam_dma_write(uint8_t data) { OAM[OAM_ADDR++] = data; }

void PPU_2C02::INCREMENT_SCROLL_X() {
  if (!(MASK & SHOW_BACKGROUND_BIT)) {
    return;
  }

  if ((VRAM_ADDR & COARSE_X_MASK) == 31) {
    VRAM_ADDR &= COARSE_X_MASK;
    VRAM_ADDR ^= 0x0400;
  } else {
    VRAM_ADDR++;
  }
}

void PPU_2C02::INCREMENT_SCROLL_Y() {
  if (!(MASK & SHOW_BACKGROUND_BIT)) {
    return;
  }

  if ((VRAM_ADDR & FINE_Y_MASK) != FINE_Y_MASK) {
    VRAM_ADDR += 0x1000;
    return;
  }

  VRAM_ADDR &= FINE_Y_MASK;
  uint16_t coarse_y = (VRAM_ADDR & COARSE_Y_MASK) >> 5;
  if (coarse_y == 29) {
    coarse_y = 0;
    VRAM_ADDR ^= 0x0800;
  } else if (coarse_y == 31) {
    coarse_y = 0;
  } else {
    coarse_y++;
  }

  VRAM_ADDR = (VRAM_ADDR & COARSE_Y_MASK) | (coarse_y << 5);
}

void PPU_2C02::TRANSFER_ADDRESS_X() {
  if (!(MASK & SHOW_BACKGROUND_BIT)) {
    return;
  }

  const uint16_t HORIZONTAL_SCROLL_MASK = (0x0400 | COARSE_X_MASK);
  VRAM_ADDR = (VRAM_ADDR & HORIZONTAL_SCROLL_MASK) |
              (TEMP_ADDR & (0x0400 & COARSE_X_MASK));
}

void PPU_2C02::TRANSFER_ADDRESS_Y() {
  if (!(MASK & SHOW_BACKGROUND_BIT)) {
    return;
  }

  VRAM_ADDR = (VRAM_ADDR & ~(0x0800 | COARSE_Y_MASK | FINE_Y_MASK)) |
              (TEMP_ADDR & (0x0800 | COARSE_Y_MASK | FINE_Y_MASK));
}

void PPU_2C02::LOAD_BACKGROUND_SHIFTERS() {
  BG_PATTERN_SHIFT_LO = (BG_PATTERN_SHIFT_LO & 0xFF00) | NEXT_TILE_LSB;
  BG_PATTERN_SHIFT_HI = (BG_PATTERN_SHIFT_HI & 0xFF00) | NEXT_TILE_MSB;

  const uint8_t palette_bits = NEXT_TILE_ATTR & 0x03;
  BG_ATTR_SHIFT_LO =
      (BG_ATTR_SHIFT_LO & 0xFF00) | ((palette_bits & 0x01) ? 0xFF : 0x00);
  BG_ATTR_SHIFT_HI =
      (BG_ATTR_SHIFT_HI & 0xFF00) | ((palette_bits & 0x02) ? 0xFF : 0x00);
}

void PPU_2C02::UPDATE_BACKGROUND_SHIFTERS() {
  if (!(MASK & SHOW_BACKGROUND_BIT)) {
    return;
  }

  BG_PATTERN_SHIFT_LO <<= 1;
  BG_PATTERN_SHIFT_HI <<= 1;
  BG_ATTR_SHIFT_LO <<= 1;
  BG_ATTR_SHIFT_HI <<= 1;
}

PPU_2C02::BACKGROUND_PIXEL PPU_2C02::GEN_BACKGROUND_PIXEL() const {
  BACKGROUND_PIXEL result{};

  const uint16_t bit_mux = (0x8000 >> FINE_X);
  const uint8_t pixel_lo = (BG_PATTERN_SHIFT_LO & bit_mux) ? 1 : 0;
  const uint8_t pixel_hi = (BG_PATTERN_SHIFT_HI & bit_mux) ? 1 : 0;
  result.pixel = (pixel_hi << 1) | pixel_lo;

  const uint8_t attr_lo = (BG_ATTR_SHIFT_LO & bit_mux) ? 1 : 0;
  const uint8_t attr_hi = (BG_ATTR_SHIFT_HI & bit_mux) ? 1 : 0;
  result.palette_select = (attr_hi << 1) | attr_lo;

  if (result.pixel == 0) {
    result.color_idx = palette_read(PALETTE_BASE);
    return result;
  }

  result.color_idx =
      palette_read(PALETTE_BASE + (result.palette_select) + result.pixel);
  return result;
}

uint8_t PPU_2C02::SPRITE_HEIGHT() const {
  return (CTRL & SPRITE_SIZE_BIT) ? 16 : 8;
}

uint8_t PPU_2C02::FETCH_SPRITE_PATTERN_BYTE(uint8_t tile, uint8_t attr,
                                            uint8_t row, bool high_plane) {}

void PPU_2C02::EVALUATE_SPRITES_FOR_SCANLINE(int target_scanline) {}

PPU_2C02::SPRITE_PIXEL PPU_2C02::GEN_SPRITE_PIXEL(int x) const {}

uint8_t PPU_2C02::cpu_read(uint16_t addr) {}

void PPU_2C02::cpu_write(uint16_t addr, uint8_t data) {}

void PPU_2C02::step() {}
