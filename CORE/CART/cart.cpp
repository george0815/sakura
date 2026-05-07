#include "cart.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

bool PARSE_FILE(string filename, CART &cartridge) {

  // Read file using file name
  ifstream file(filename, ios::binary); // read file as binary

  // Create vector of bytes
  vector<uint8_t> data((istreambuf_iterator<char>(file)),
                       istreambuf_iterator<char>());

  // Validate file size
  if (data.size() < NES_HEADER_SIZE) {
    return false;
  }

  // Copy the first 16 bytes into the header file
  memcpy(&cartridge.HEADER, data.data(), NES_HEADER_SIZE);

  // Validate header (if first 4 bytes are ASCII "NES" and MSDOS EOF)
  if (cartridge.HEADER.NAME[0] != 'N' || cartridge.HEADER.NAME[1] != 'E' ||
      cartridge.HEADER.NAME[2] != 'S' || cartridge.HEADER.NAME[3] != 0x1A) {
    return false;
  }

  // Calculate PRG and CHR sizes, the iNES stores it as a count of chunks
  const size_t prg_size = cartridge.HEADER.PRG_CHUNKS * PRG_BANK_CHUNK_SIZE;
  const size_t chr_size = cartridge.HEADER.CHR_CHUNKS * CHR_CHUNK_SIZE;

  // Sets pointer to move through data
  size_t offset = NES_HEADER_SIZE; // Already parsed header so start after
                                   // header

  // Check if reading past EOF
  if (offset + prg_size + chr_size > data.size()) {
    return false;
  }

  // Sets PRG ROM
  cartridge.PRG.assign(data.begin() + offset, data.begin() + offset + prg_size);

  // Increment offset to move through data
  offset += prg_size;

  // CHR handling
  if (chr_size == 0) {
    cartridge.USES_CHR_RAM = true; // CHR ROM is 0, so use CHR RAM
    cartridge.CHR.assign(CHR_CHUNK_SIZE, 0);
  } else {
    cartridge.USES_CHR_RAM = false; // Use CHR ROM
    cartridge.CHR.assign(data.begin() + offset,
                         data.begin() + offset + chr_size);
  }

  // For testing TODO REMOVE LATER
  cout << "NES\n";
  return true;

  // Parse flags, this is mainly for the PPU
  const bool vert = cartridge.HEADER.FLAGS_6 & 0x01;
  cartridge.USES_BATTERY_BACKED_SRAM = (cartridge.HEADER.FLAGS_6 & 0x02) != 0;
  const bool four = cartridge.HEADER.FLAGS_6 & 0x08;

  cartridge.MIRRORING =
      four ? MIRRORING::FOUR_SCREEN
           : (vert ? MIRRORING::VERTICAL : MIRRORING::HORIZONTAL);

  cartridge.PRG_RAM_SIZE =
      (cartridge.HEADER.FLAGS_8
           ? cartridge.HEADER.FLAGS_8 * PRG_RAM_SIZE
           : (cartridge.USES_BATTERY_BACKED_SRAM ? PRG_RAM_SIZE : 0));

  cartridge.MAPPER_ID =
      ((cartridge.HEADER.FLAGS_6 >> 4) | (cartridge.HEADER.FLAGS_7 & 0xF0));
  return true;
}
