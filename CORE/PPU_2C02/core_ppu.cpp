#include "./core_ppu.h"
#include "../CPU_6502/core.h"
#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <iostream>
#include <pthread.h>
#include <string>
#include <sys/types.h>

using namespace std;

const uint32_t NES_PALETTE[64] = {
    0x666666, 0x002A88, 0x1412A7, 0x3B00A4, 0x5C007E, 0x6E0040, 0x6C0600,
    0x561D00, 0x333500, 0x0B4800, 0x005200, 0x004F08, 0x00404D, 0x000000,
    0x000000, 0x000000, 0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE, 0xA01ACC,
    0xB71E7B, 0xB53120, 0x994E00, 0x6B6D00, 0x388700, 0x0E9300, 0x008F32,
    0x007C8D, 0x000000, 0x000000, 0x000000, 0xFFFEFF, 0x64B0FF, 0x9290FF,
    0xC676FF, 0xF36AFF, 0xFE6ECC, 0xFE8170, 0xEA9E22, 0xBCBE00, 0x88D800,
    0x5CE430, 0x45E082, 0x48CDDE, 0x4F4F4F, 0x000000, 0x000000, 0xFFFEFF,
    0xC0DFFF, 0xD3D2FF, 0xE8C8FF, 0xFBC2FF, 0xFEC4EA, 0xFECCC5, 0xF7D8A5,
    0xE4E594, 0xCFEE96, 0xBDF4AB, 0xB3F3CC, 0xB5EBF2, 0xB8B8B8, 0x000000,
    0x000000

};

PPU_2C02::PPU_2C02() {
  PALETTE.fill(0);
  NAMETABLES.fill(0);
  OAM.fill(0);
  FRAMEBUFFER.fill(0xFFFFFFFF);
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
    return (table * NAMETABLE_SIZE + offset);
    break;

  case MIRRORING::SINGLE_SCREEN_LOWER:
    table = 0;
    break;

  case MIRRORING::SINGLE_SCREEN_UPPER:
    table = 1;
    break;
  }
  return (table * NAMETABLE_SIZE + offset);
}

uint8_t PPU_2C02::ppu_read(uint16_t addr) {
  addr &= 0x3FFF;
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
  if (!(MASK & (SHOW_BACKGROUND_BIT | SHOW_SPRITES_BIT))) {
    return;
  }

  if ((VRAM_ADDR & COARSE_X_MASK) == 31) {
    VRAM_ADDR &= ~COARSE_X_MASK;
    VRAM_ADDR ^= 0x0400;
  } else {
    VRAM_ADDR++;
  }
}

void PPU_2C02::INCREMENT_SCROLL_Y() {
  if (!(MASK & (SHOW_BACKGROUND_BIT | SHOW_SPRITES_BIT))) {
    return;
  }

  if ((VRAM_ADDR & FINE_Y_MASK) != FINE_Y_MASK) {
    VRAM_ADDR += 0x1000;
    return;
  }

  VRAM_ADDR &= ~FINE_Y_MASK;
  uint16_t coarse_y = (VRAM_ADDR & COARSE_Y_MASK) >> 5;
  if (coarse_y == 29) {
    coarse_y = 0;
    VRAM_ADDR ^= 0x0800;
  } else if (coarse_y == 31) {
    coarse_y = 0;
  } else {
    coarse_y++;
  }

  VRAM_ADDR = (VRAM_ADDR & ~COARSE_Y_MASK) | (coarse_y << 5);
}

void PPU_2C02::TRANSFER_ADDRESS_X() {
  if (!(MASK & (SHOW_BACKGROUND_BIT | SHOW_SPRITES_BIT))) {
    return;
  }

  const uint16_t HORIZONTAL_SCROLL_MASK = (0x0400 | COARSE_X_MASK);
  VRAM_ADDR = (VRAM_ADDR & ~HORIZONTAL_SCROLL_MASK) |
              (TEMP_ADDR & (0x0400 | COARSE_X_MASK));
}

void PPU_2C02::TRANSFER_ADDRESS_Y() {
  if (!(MASK & (SHOW_BACKGROUND_BIT | SHOW_SPRITES_BIT))) {
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
  if (!(MASK & (SHOW_BACKGROUND_BIT | SHOW_SPRITES_BIT))) {
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
      palette_read(PALETTE_BASE + (result.palette_select << 2) + result.pixel);
  return result;
}

uint8_t PPU_2C02::SPRITE_HEIGHT() const {
  return (CTRL & SPRITE_SIZE_BIT) ? 16 : 8;
}

uint8_t PPU_2C02::FETCH_SPRITE_PATTERN_BYTE(uint8_t tile, uint8_t attr,
                                            uint8_t row, bool high_plane) {
  uint8_t fine_y = row;
  uint16_t addr = 0;

  if (attr & SPRITE_VERTICAL_FLIP_BIT) {
    fine_y = (SPRITE_HEIGHT() - 1 - fine_y);
  }

  if (SPRITE_HEIGHT() == 16) {
    const uint16_t table = (tile & 0x01) ? 0x1000 : 0x0000;
    const uint8_t tile_index = (tile & 0xFE);
    if (fine_y >= 8) {
      fine_y -= 8;
      addr = table + (tile_index + 1) * PATTERN_TABLE_STRIDE + fine_y;
    } else {
      addr = table + tile_index * PATTERN_TABLE_STRIDE + fine_y;
    }
  } else {
    const uint16_t table = (CTRL & SPRITE_TABLE_BIT) ? 0x1000 : 0x0000;
    addr = table + tile * PATTERN_TABLE_STRIDE + fine_y;
  }
  if (high_plane) {
    addr += PATTERN_PLANE_OFFSET;
  }
  return ppu_read(addr);
}

void PPU_2C02::EVALUATE_SPRITES_FOR_SCANLINE(int target_scanline) {
  SECONDARY_OAM_COUNT = 0;
  for (auto &sprite : SECONDARY_OAM) {
    sprite = {};
  }

  if (target_scanline < 0 || target_scanline >= FRAME_HEIGHT) {
    return;
  }

  const uint8_t height = SPRITE_HEIGHT();

  bool overflow = false;

  for (int i = 0; i < 64; i++) {
    const uint8_t y = OAM[i * 4 + 0];
    const int sprite_top = y + 1;
    const int row = target_scanline - sprite_top;
    if (row < 0 || row >= height) {
      continue;
    }

    if (SECONDARY_OAM_COUNT < SECONDARY_OAM.size()) {
      SPRITE_ENTRY &entry = SECONDARY_OAM[SECONDARY_OAM_COUNT++];
      entry.y = y;
      entry.tile = OAM[i * 4 + 1];
      entry.attr = OAM[i * 4 + 2];
      entry.x = OAM[i * 4 + 3];

      entry.index = i;
      entry.pattern_lo =
          FETCH_SPRITE_PATTERN_BYTE(entry.tile, entry.attr, row, false);
      entry.pattern_hi =
          FETCH_SPRITE_PATTERN_BYTE(entry.tile, entry.attr, row, true);

    } else {
      overflow = true;
      break;
    }
  }
  if (overflow) {
    STATUS |= SPRITE_OVERFLOW_BIT;
  }
}

PPU_2C02::SPRITE_PIXEL PPU_2C02::GEN_SPRITE_PIXEL(int x) const {
  SPRITE_PIXEL result{};

  if (!(MASK & SHOW_SPRITES_BIT)) {
    return result;
  }

  if ((MASK & SHOW_SPRITES_LEFT_EDGE_BIT) == 0 && x < 8) {
    return result;
  }

  for (uint8_t i = 0; i < SECONDARY_OAM_COUNT; i++) {
    const SPRITE_ENTRY &entry = SECONDARY_OAM[i];
    if (x < entry.x || x >= entry.x + 8) {
      continue;
    }

    int pixel_x = x - entry.x;
    if ((entry.attr & SPRITE_HORIZONTAL_FLIP_BIT) == 0) {
      pixel_x = 7 - pixel_x;
    }

    const uint8_t pixel_lo = (entry.pattern_lo >> pixel_x) & 0x01;
    const uint8_t pixel_hi = (entry.pattern_hi >> pixel_x) & 0x01;
    const uint8_t pixel = (pixel_hi << 1) | pixel_lo;

    if (pixel == 0) {
      continue;
    }

    result.pixel = pixel;
    result.palette_select = entry.attr & 0x03;
    result.priority_behind_background = (entry.attr & SPRITE_PRIORITY_BIT) != 0;
    result.is_sprite_zero = entry.index == 0;
    return result;
  }
  return result;
}

uint8_t PPU_2C02::cpu_read(uint16_t addr) {
  switch (addr & 0x2007) {

  case 0x2002: {
    uint8_t result = STATUS;
    STATUS &= ~VBLANK_BIT;
    ADDR_LATCH = false;
    return result;
  }
  case 0x2004: {
    return OAM[OAM_ADDR];
  }
  case 0x2007: {
    const uint16_t paddr = VRAM_ADDR;
    const uint8_t data = ppu_read(paddr);
    VRAM_ADDR += (CTRL & VRAM_INCREMENT_BIT) ? 32 : 1;
    uint8_t ret = BUFFERED_DATA;
    BUFFERED_DATA = data;
    if (paddr >= PALETTE_BASE) {
      ret = data;
    }
    return ret;
  }
  default: {
    return 0;
  }
  }
}

void PPU_2C02::cpu_write(uint16_t addr, uint8_t data) {

  if (addr == 0x2000 || addr == 0x2001 || addr == 0x2005 || addr == 0x2006) {
    cout << "PPU write $" << hex << addr << " = $" << (int)data
         << " at scanline=" << dec << SCANLINE << " cycle=" << CYCLES << endl;
  }

  switch (addr & 0x2007) {
  case 0x2000:
    CTRL = data;
    TEMP_ADDR = (TEMP_ADDR & ~NAMETABLE_BITS_MASK) | ((data & 0x03) << 10);
    break;
  case 0x2001:
    MASK = data;
    break;
  case 0x2003:
    OAM_ADDR = data;
    break;
  case 0x2004:
    OAM[OAM_ADDR++] = data;
    break;
  case 0x2005:
    if (!ADDR_LATCH) {
      FINE_X = data & 0x07;
      TEMP_ADDR = (TEMP_ADDR & ~COARSE_X_MASK) | (data >> 3);
      ADDR_LATCH = true;
    } else {
      TEMP_ADDR = (TEMP_ADDR & ~(COARSE_Y_MASK | FINE_Y_MASK)) |
                  ((data & 0x07) << 12) | ((data & 0xF8) << 2);
      ADDR_LATCH = false;
    }
    break;
  case 0x2006:
    if (!ADDR_LATCH) {
      TEMP_ADDR = (TEMP_ADDR & 0x00FF) | ((data & 0x3F) << 8);
      ADDR_LATCH = true;
    } else {
      TEMP_ADDR = (TEMP_ADDR & 0x7F00) | data;
      VRAM_ADDR = TEMP_ADDR;
      ADDR_LATCH = false;
    }
    break;
  case 0x2007:
    ppu_write(VRAM_ADDR, data);
    VRAM_ADDR += (CTRL & VRAM_INCREMENT_BIT) ? 32 : 1;
    break;
  default:
    break;
  }
}

void PPU_2C02::step() {

  // cout << "SCANLINE: " << to_string(SCANLINE) << endl;
  // cout << "CYCLES: " << to_string(CYCLES) << endl;

  const bool pre_render_line = (SCANLINE == 261);
  const bool visible_line = (SCANLINE >= 0 && SCANLINE < FRAME_HEIGHT);
  const bool render_line = visible_line || pre_render_line;
  const bool visible_cycle = (CYCLES >= 1 && CYCLES <= FRAME_WIDTH);
  const bool fetch_cycle = ((CYCLES >= 1 && CYCLES <= FRAME_WIDTH) ||
                            (CYCLES >= 321 && CYCLES <= 336));

  if (visible_line && visible_cycle) {
    const int x = CYCLES - 1;
    const int y = SCANLINE;

    const bool background_hidden =
        !(MASK & SHOW_BACKGROUND_BIT) ||
        ((MASK & SHOW_BACKGROUND_LEFT_EDGE_BIT) == 0 && x < 8);

    const BACKGROUND_PIXEL bg =
        background_hidden ? BACKGROUND_PIXEL{0, 0, palette_read(PALETTE_BASE)}
                          : GEN_BACKGROUND_PIXEL();

    const SPRITE_PIXEL sprite = GEN_SPRITE_PIXEL(x);

    uint8_t color_idx = bg.color_idx;

    if (sprite.pixel != 0) {
      const uint8_t sprite_color =
          palette_read(0x3F10 + (sprite.palette_select << 2) + sprite.pixel);

      if (bg.pixel == 0 || !sprite.priority_behind_background) {
        color_idx = sprite_color;
      }

      if (sprite.is_sprite_zero && bg.pixel != 0 && sprite.pixel != 0) {
        STATUS |= SPRITE_ZERO_HIT_BIT;
      }
    }
    FRAMEBUFFER[y * FRAME_WIDTH + x] =
        0xFF000000 | NES_PALETTE[color_idx & 0x3F];
  }

  if (render_line && fetch_cycle) {
    UPDATE_BACKGROUND_SHIFTERS();

    switch ((CYCLES - 1) & 0x07) {
    case 0: {
      LOAD_BACKGROUND_SHIFTERS();
      NEXT_TILE_ID = ppu_read(NAMETABLE_BASE | (VRAM_ADDR & 0x0FFF));
      break;
    }

    case 2: {
      const uint16_t attr_addr =
          NAMETABLE_BASE | ATTRIBUTE_TABLE_OFFSET | (VRAM_ADDR & 0x0C00) |
          ((VRAM_ADDR >> 4) & 0x38) | ((VRAM_ADDR >> 2) & 0x07);

      uint8_t attr = ppu_read(attr_addr);
      if (VRAM_ADDR & 0x0002) {
        attr >>= 2;
      }
      if (VRAM_ADDR & 0x0040) {
        attr >>= 4;
      }
      NEXT_TILE_ATTR = attr & 0x03;
      break;
    }
    case 4: {
      const uint16_t fine_y = (VRAM_ADDR >> 12) & 0x07;
      const uint16_t pattern_base =
          (CTRL & BACKGROUND_TABLE_BIT) ? 0x1000 : 0x0000;
      const uint16_t tile_addr =
          pattern_base + NEXT_TILE_ID * PATTERN_TABLE_STRIDE + fine_y;
      NEXT_TILE_LSB = ppu_read(tile_addr);
      break;
    }
    case 6: {
      const uint16_t fine_y = (VRAM_ADDR >> 12) & 0x07;
      const uint16_t pattern_base =
          (CTRL & BACKGROUND_TABLE_BIT) ? 0x1000 : 0x0000;
      const uint16_t tile_addr =
          pattern_base + NEXT_TILE_ID * PATTERN_TABLE_STRIDE + fine_y;
      NEXT_TILE_MSB = ppu_read(tile_addr + PATTERN_PLANE_OFFSET);
      break;
    }
    case 7: {
      INCREMENT_SCROLL_X();
      break;
    }
    default:
      break;
    }
  }

  if (render_line) {
    if (CYCLES == 256) {
      INCREMENT_SCROLL_Y();
    } else if (CYCLES == 257) {
      LOAD_BACKGROUND_SHIFTERS();
      TRANSFER_ADDRESS_X();
      if (visible_line) {
        EVALUATE_SPRITES_FOR_SCANLINE(SCANLINE + 1);
      } else if (pre_render_line) {
        EVALUATE_SPRITES_FOR_SCANLINE(0);
      }
    } else if (pre_render_line && CYCLES >= 280 && CYCLES <= 304) {
      TRANSFER_ADDRESS_Y();
    }

    if (visible_line && CYCLES == 270 && MAPPER &&
        (MASK & (SHOW_BACKGROUND_BIT | SHOW_SPRITES_BIT))) {
      static int clock_count = 0;
      if (clock_count++ < 100) {
        cout << "MMC3 SCANLINE CLOCK: " << "SCANLINE=" << SCANLINE
             << " CYCLE=" << CYCLES << " mask=" << hex << int(MASK) << dec
             << endl;
      }
      MAPPER->on_ppu_scanline(CPU);
    }
  }

  if (pre_render_line && CYCLES == 1) {
    STATUS &= (~(VBLANK_BIT | SPRITE_OVERFLOW_BIT | SPRITE_ZERO_HIT_BIT));
    FRAME_DONE = false;
  }

  CYCLES++;

  if (CYCLES >= PPU_CYCLES_PER_LINE) {
    CYCLES = 0;
    // cout << "CYCLES CLEAR" << endl;
    SCANLINE++;
  }

  if (SCANLINE == 241 && CYCLES == 1) {
    STATUS |= VBLANK_BIT;
    // cout << "VBLANK SET" << endl;
    FRAME_DONE = true;
    if (CPU && (CTRL & NMI_ENABLE_BIT)) {
      CPU->NMI_HANDLER();
    }
  }

  if (SCANLINE == 261 && CYCLES == 1) {
    STATUS &= ~VBLANK_BIT;
    //  cout << "VBLANK CLEAR" << endl;
  }

  if (SCANLINE >= PPU_SCANLINES) {
    SCANLINE = 0;
  }
}
