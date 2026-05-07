#pragma once

#include <array>
#include <cstdint>
#include <vector>

using namespace std;

class CPU_6502;

enum MIRRORING {
  HORIZONTAL,
  VERTICAL,
  FOUR_SCREEN,
  SINGLE_SCREEN_LOWER,
  SINGLE_SCREEN_UPPER
};

class Mapper {
public:
  // CONSTANTS
  const uint16_t PRG_BASE = 0x8000;
  const size_t PRG_SINGLE_BANK_SIZE = 0x4000;
  const uint16_t PATTERN_TABLE_END = 0x2000;

  virtual ~Mapper() = default;

  virtual uint8_t cpu_read(uint16_t addr) = 0;
  virtual void cpu_write(uint16_t addr, uint8_t data) = 0;

  virtual uint8_t ppu_read(uint16_t addr) = 0;
  virtual void ppu_write(uint16_t addr, uint8_t data) = 0;

  virtual MIRRORING mirroring_mode() const = 0;
  virtual void on_ppu_scanline(CPU_6502 *cpu) { (void)cpu; }
};

class Mapper0 : public Mapper {

public:
  Mapper0(vector<uint8_t> &prg, vector<uint8_t> &chr, bool chr_is_ram,
          MIRRORING mirroring);

  uint8_t cpu_read(uint16_t addr) override;
  void cpu_write(uint16_t addr, uint8_t data) override;

  uint8_t ppu_read(uint16_t addr) override;
  void ppu_write(uint16_t addr, uint8_t data) override;

  MIRRORING mirroring_mode() const override { return MIRROR; };

private:
  const vector<uint8_t> &PRG_ROM;
  vector<uint8_t> &CHR_MEM;
  bool CHR_IS_RAM;
  MIRRORING MIRROR;
};
