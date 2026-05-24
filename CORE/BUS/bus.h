#pragma once

#include "../../MAIN/save.h"
#include "../APU_2A03/wrapper.h"
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
class APU_2A03;

class BUS {

public:
  enum CONTROLLER_BUTTON {

    A = 0,
    B = 1,
    SELECT = 2,
    START = 3,
    UP = 4,
    DOWN = 5,
    LEFT = 6,
    RIGHT = 7

  };

  void SET_CONTROLLER_BUTTON(int port, CONTROLLER_BUTTON button, bool pressed);
  void save_state(StateWriter &writer) const;
  bool load_state(StateReader &reader);

  bool CONTROLLER_STROBE = false;
  array<uint8_t, 2> CONTROLLER_STATE{};
  array<uint8_t, 2> CONTROLLER_SHIFT{};

  // CPU has access to 2KB of internal RAM, its fixed so I'll be using an array
  array<uint8_t, 0x0800> CPU_RAM;

  // Program ROM can either be 16KB or 32KB, so I'll be using a vector
  vector<uint8_t> PRG_ROM;

  vector<uint8_t> CHR_MEM;

  vector<uint8_t> PRG_RAM;

  array<uint8_t, 0x8000> SHADOW{};

  // INIT/FUNCTIONING
  void insert_cartridge(CART &cart);
  void connect_cpu(CPU_6502 &cpu);
  void connect_ppu(PPU_2C02 &ppu);
  ~BUS();

  // READ/WRITE
  uint8_t read(uint16_t addr);
  void write(uint16_t addr, uint8_t data);

  // COMPONENTS
  CPU_6502 *CPU;
  PPU_2C02 *PPU;
  Mapper *MAPPER;
  mutable APU_2A03 APU;

  // METADATA
  bool HAS_PRG = false;
  bool CHR_IS_RAM = false;
  bool BATTERY_BACKED = false;
  uint16_t MAPPER_ID = 0;
  uint32_t CART_SIGN = 0;
  MIRRORING MIRROR_MODE = HORIZONTAL;
  bool HAS_BATTERY_BACKED_SRAM() { return BATTERY_BACKED && !PRG_RAM.empty(); };

  bool LOAD_BATTERY_BACKED_SRAM(const vector<uint8_t> &data);

  void END_AUDIO_FRAME(uint64_t cpu_cycle);

  vector<int16_t> TAKE_AUDIO_SAMPLES();
};
