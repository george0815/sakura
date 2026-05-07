#pragma once

#include "../CART/cart.h"
#include "../CPU_6502/core.h"
#include "../MAPPERS/mapper.h"
#include "../PPU_2C02/core_ppu.h"
#include <array>
#include <cstdint>
#include <vector>

struct CART;

class CPU_6502;
class PPU_2C02;

class BUS {

public:
  // CPU has access to 2KB of internal RAM, its fixed so I'll be using an array
  std::array<uint8_t, 0x0800> CPU_RAM;

  // Program ROM can either be 16KB or 32KB, so I'll be using a vector
  std::vector<uint8_t> PRG_ROM;

  // INIT/FUNCTIONING
  void insert_cartridge(CART &cart);
  void connect_cpu(CPU_6502 &cpu);
  void connect_ppu(PPU_2C02 &ppu);
  void step();
  // READ/WRITE
  uint8_t read(uint16_t addr);
  void write(uint16_t addr, uint8_t data);

  // COMPONENTS
  CPU_6502 *CPU;
  PPU_2C02 *PPU;
  Mapper *MAPPER;

  // METADATA
  bool HAS_PRG = false;
  bool CHR_IS_RAM = false;
  bool BATTERY_BACKED = false;
  uint16_t MAPPER_ID = 0;
  uint32_t CART_SIGN = 0;
  MIRRORING MIRRORING = HORIZONTAL;
};
