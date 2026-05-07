#pragma once

#include "../MAPPERS/mapper.h"
#include <array>
#include <cstdint>
#include <sys/types.h>

using namespace std;

class PPU_2C02 {
public:
  // CONSTANTS
  const int FRAME_WIDTH = 256;
  const int FRAME_HEIGHT = 240;
  const int TOTAL_PIXELS = FRAME_HEIGHT * FRAME_WIDTH;

  PPU_2C02();

  void connect_mapper(Mapper *mapper, MIRRORING mirroring);
  void connect_cpu(class CPU_6502 *cpu_ptr) { cpu = cpu_ptr; }

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
  const uint16_t KB_8 = 0x2000; // 8 KB
  const uint16_t PALETTE_BASE = 0x3F00;
  const uint16_t PPU_ADDRESS_RANGE_END = 0x4000;
  const uint16_t NAMETABLE_REGION_MASK = 0x0FFF;
  const uint16_t NAMETABLE_SIZE = 0x0400;
  const uint16_t FOUR_SCREEN_NAMETABLE_BYTES = 0x1000;
  const uint16_t BIT_2 = 0x04;
  const uint16_t BIT_3 = 0x08;
  const uint16_t BIT_4 = 0x10;
  const uint16_t BIT_5 = 0x20;
  const uint16_t BIT_6 = 0x40;
  const uint16_t BIT_7 = 0x80;
  const uint16_t SHOW_BACKGROUND_LEFT_EDGE_BIT = 0x02;
  const uint16_t COARSE_X_MASK = 0x001F;
  const uint16_t COARSE_Y_MASK = 0x03E0;
  const uint16_t NAMETABLE_BITS_MASK = 0x0C00;
  const uint16_t FINE_MASK = 0x7000;
  const uint16_t UNUSED_VRAM_MASK = 0x7FFF;
  const uint16_t ATTRIBUTE_TABLE_OFFSET = 0x03C0;
  const uint16_t PATTERN_TABLE_STRIDE = 16;
  const uint16_t PATTERN_PLANE_OFFSET = 8;
};
