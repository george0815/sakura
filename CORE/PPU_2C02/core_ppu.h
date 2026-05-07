#pragma once

#include "../MAPPERS/mapper.h"
#include <array>
#include <cstdint>
#include <sys/types.h>

using namespace std;

class PPU_2C02 {
public:
  // CONSTANTS
  const static int FRAME_WIDTH = 256;
  const static int FRAME_HEIGHT = 240;
  const static int TOTAL_PIXELS = FRAME_HEIGHT * FRAME_WIDTH;

  PPU_2C02();

  void connect_mapper(Mapper *mapper, MIRRORING mirroring);
  void connect_cpu(class CPU_6502 *cpu_ptr) { CPU = cpu_ptr; }

  uint8_t cpu_read(uint16_t addr, uint8_t data);
  void cpu_write(uint16_t addr, uint8_t data);
  void oam_dma_write(uint8_t data);

  uint8_t ppu_read(uint16_t addr);
  void ppu_write(uint16_t addr, uint8_t data);

  const array<uint32_t, TOTAL_PIXELS> &FRAMEBUFFER_DATA() const {
    return FRAMEBUFFER;
  }

  void step();
  bool FRAME_COMPLETE() const { return FRAME_DONE; }
  void CLEAR_FRAME_FLAG() { FRAME_DONE = false; }
  const array<uint8_t, 0x20> &PALETTE_DATA() const { return PALETTE; }

private:
  const uint16_t PATTERN_TABLE_END = 0x2000;
  const uint16_t NAMETABLE_BASE = 0x2000;
  const uint16_t PALETTE_BASE = 0x3F00;
  const uint16_t PPU_ADDRESS_RANGE_END = 0x4000;
  const uint16_t NAMETABLE_REGION_MASK = 0x0FFF;
  const uint16_t NAMETABLE_SIZE = 0x0400;
  const static uint16_t FOUR_SCREEN_NAMETABLE_BYTES = 0x1000;
  const uint16_t PALETTE_MIRROR_SIZE = 0x20;
  const uint16_t VBLANK_BIT = 0x80;
  const uint16_t VRAM_INCREMENT_BIT = 0x04;
  const uint16_t NMI_ENABLE_BIT = 0x80;
  const uint16_t BACKGROUND_TABLE_BIT = 0x10;
  const uint16_t SPRITE_TABLE_BIT = 0x08;
  const uint16_t SHOW_BACKGROUND_BIT = 0x08;
  const uint16_t SHOW_BACKGROUND_LEFT_EDGE_BIT = 0x02;
  const uint16_t SHOW_SPRITES_BIT = 0x10;
  const uint16_t SHOW_SPRITES_LEFT_EDGE_BIT = 0x04;
  const uint16_t SPRITE_SIZE_BIT = 0x20;
  const uint16_t SPRITE_PRIORITY_BIT = 0x20;
  const uint16_t SPRITE_HORIZONTAL_FLIP_BIT = 0x40;
  const uint16_t SPRITE_VERTICAL_FLIP_BIT = 0x80;
  const uint16_t SPRITE_OVERFLOW_BIT = 0x20;
  const uint16_t SPRITE_ZERO_HIT_BIT = 0x40;
  const uint16_t COARSE_X_MASK = 0x001F;
  const uint16_t COARSE_Y_MASK = 0x03E0;
  const uint16_t NAMETABLE_BITS_MASK = 0x0C00;
  const uint16_t FINE_Y_MASK = 0x7000;
  const uint16_t UNUSED_VRAM_MASK = 0x7FFF;
  const uint16_t ATTRIBUTE_TABLE_OFFSET = 0x03C0;
  const uint16_t PATTERN_TABLE_STRIDE = 16;
  const uint16_t PATTERN_PLANE_OFFSET = 8;

  Mapper *MAPPER;
  class CPU_6502 *CPU;
  MIRRORING MIRROR_MODE;

  uint8_t CTRL = 0;
  uint8_t MASK = 0;
  uint8_t STATUS = 0xA0;
  uint8_t OAM_ADDR = 0;
  uint8_t OAM_DATA = 0;
  uint8_t FINE_X = 0;
  uint16_t VRAM_ADDR = 0;
  uint16_t TEMP_ADDR = 0;
  bool ADDR_LATCH = false;
  bool FRAME_DONE = false;
  int SCANLINE = 0;
  int CYCLES = 0;

  array<uint8_t, FOUR_SCREEN_NAMETABLE_BYTES> NAMETABLES{};
  array<uint8_t, 0x20> PALETTE{};
  array<uint8_t, 256> OAM{};
  array<uint32_t, TOTAL_PIXELS> FRAMEBUFFER{};

  uint8_t NEXT_TILE_ID = 0;
  uint8_t NEXT_TILE_ATTR = 0;
  uint8_t NEXT_TILE_LSB = 0;
  uint8_t NEXT_TILE_MSB = 0;
  uint16_t BG_PATTERN_SHIFT_LO = 0;
  uint16_t BG_PATTERN_SHIFT_HI = 0;
  uint16_t BG_ATTR_SHIFT_LO = 0;
  uint16_t BG_ATTR_SHIFT_HI = 0;

  struct SPRITE_ENTRY {
    uint8_t y = 0xFF;
    uint8_t tile = 0;
    uint8_t attr = 0;
    uint8_t x = 0xFF;
    uint8_t index = 0xFF;
    uint8_t pattern_lo = 0;
    uint8_t pattern_hi = 0;
  };

  struct BACKGROUND_PIXEL {
    uint8_t pixel = 0;
    uint8_t palette_select = 0;
    uint8_t color_idx = 0;
  };

  struct SPRITE_PIXEL {
    uint8_t pixel = 0;
    uint8_t palette_select = 0;
    bool priority_behind_background = false;
    bool is_sprite_zero = false;
  };

  array<SPRITE_ENTRY, 8> SECONDARY_OAM{};
  uint8_t SECONDARY_OAM_COUNT = 0;

  uint16_t MIRROR_ADDR(uint16_t addr) const;
  uint8_t palette_read(uint16_t addr) const;
  void palette_write(uint16_t addr, uint8_t data);
  void INCREMENT_SCROLL_X();
  void INCREMENT_SCROLL_Y();
  void TRANSFER_ADDRESS_X();
  void TRANSFER_ADDRESS_Y();
  void LOAD_BACKGROUND_SHIFTERS();
  void UPDATE_BACKGROUND_SHIFTERS();
  BACKGROUND_PIXEL GEN_BACKGROUND_PIXEL() const;
  uint8_t SPRITE_HEIGHT() const;
  uint8_t FETCH_SPRITE_PATTERN_BYTE(uint8_t tile, uint8_t attr, uint8_t row,
                                    bool high_plane);
  void EVALUATE_SPRITES_FOR_SCANLINE(int target_scanline);
  SPRITE_PIXEL GEN_SPRITE_PIXEL(int x) const;
};
