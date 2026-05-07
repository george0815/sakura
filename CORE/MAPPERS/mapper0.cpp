#include "./mapper.h"
#include <cstdint>

Mapper0::Mapper0(vector<uint8_t> &prg, vector<uint8_t> &chr, bool chr_is_ram,
                 MIRRORING mirroring)
    : PRG_ROM(prg), CHR_MEM(chr), CHR_IS_RAM(chr_is_ram), MIRROR(mirroring) {}

uint8_t Mapper0::cpu_read(uint16_t addr) {
  if (addr >= PRG_BASE) {
    uint32_t offset = addr - PRG_BASE;
    if (PRG_ROM.size() == PRG_SINGLE_BANK_SIZE) {
      offset %= PRG_SINGLE_BANK_SIZE;
    } else {
      offset %= PRG_ROM.size();
    }
    return PRG_ROM[offset];
  }
  return 0;
}

void Mapper0::cpu_write(uint16_t addr, uint8_t data) {}

uint8_t Mapper0::ppu_read(uint16_t addr) {
  if (addr < PATTERN_TABLE_END) {
    return CHR_MEM[addr % CHR_MEM.size()];
  }
  return 0;
}

void Mapper0::ppu_write(uint16_t addr, uint8_t data) {
  if (CHR_IS_RAM && addr < PATTERN_TABLE_END) {
    CHR_MEM[addr % CHR_MEM.size()] = data;
  }
}
