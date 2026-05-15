#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <sys/types.h>
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
  virtual bool IS_IRQ_PENDING() { return false; }
  virtual void NOTIFY_PPU_ADDRESS(uint16_t addr) {}
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

class Mapper4 : public Mapper {

public:
  Mapper4(const vector<uint8_t> &prg, vector<uint8_t> &chr, bool chr_ram,
          MIRRORING mirroring_mode);
  uint8_t cpu_read(uint16_t addr) override;
  void cpu_write(uint16_t addr, uint8_t data) override;
  uint8_t ppu_read(uint16_t addr) override;
  void ppu_write(uint16_t addr, uint8_t data) override;
  MIRRORING mirroring_mode() const override { return mirror_mode; }
  void on_ppu_scanline(CPU_6502 *cpu) override;
  bool IS_IRQ_PENDING() override { return IRQ_PENDING; }
  bool PREV_A12 = false;
  void NOTIFY_PPU_ADDRESS(uint16_t addr);

private:
  static const size_t PRG_BANK_SIZE = 0x2000;
  static const size_t CHR_BANK_SIZE = 0x0400;

  size_t PRG_BANK_COUNT() const;
  size_t CHR_BANK_COUNT() const;
  size_t PRG_BANK_INDEX(int slot) const;
  size_t CHR_BANK_INDEX(uint16_t addr) const;
  void CLOCK_IRQ_COUNTER(CPU_6502 *cpu);

  const vector<uint8_t> &PRG_ROM;
  vector<uint8_t> &CHR_MEM;
  bool CHR_IS_RAM;
  MIRRORING mirror_mode;
  array<uint8_t, 8> BANK_REGS{};
  uint8_t BANK_SELECT = 0;
  bool PRG_MODE = false;
  bool CHR_INVERSION = false;
  uint8_t IRQ_LATCH = 0;
  uint8_t IRQ_COUNTER = 0;
  bool IRQ_PENDING = false;
  bool IRQ_RELOAD = false;
  bool IRQ_ENABLED = false;
};
