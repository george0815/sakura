#include "../PPU_2C02/core_ppu.h"
#include "logger.h"
#include <string>

using namespace std;

PPU_LOGGER::PPU_LOGGER(PPU_2C02 *ppu) {
  PPU_LOGGER::PPU = ppu;
  OUTPUT_FILE_PATH = "./ppu_log.txt";
}

void PPU_LOGGER::WRITE() {
  LOG.push_back("SCANLINE: " + to_string(PPU->SCANLINE));
  LOG.push_back("CYCLE: " + to_string(PPU->CYCLES));
  LOG.push_back("CTRL: " + to_string(PPU->CTRL));
  LOG.push_back("MASK: " + to_string(PPU->MASK));
  LOG.push_back("STATUS: " + to_string(PPU->STATUS));
  LOG.push_back("OAM_ADDR: " + to_string(PPU->OAM_ADDR));
  LOG.push_back("OAM_DATA: " + to_string(PPU->OAM_DATA));
  LOG.push_back("FINE_X: " + to_string(PPU->FINE_X));
  LOG.push_back("VRAM_ADDR: " + to_string(PPU->VRAM_ADDR));
  LOG.push_back("TEMP_ADDR: " + to_string(PPU->TEMP_ADDR));
  LOG.push_back("ADDR_LATCH: " + to_string(PPU->ADDR_LATCH));
  LOG.push_back("FRAME_DONE: " + to_string(PPU->FRAME_DONE));
  LOG.push_back("BUFFERED_DATA: " + to_string(PPU->BUFFERED_DATA));
  LOG.push_back("\n");
}
