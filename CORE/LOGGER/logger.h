#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class CPU_6502;
class PPU_2C02;

class LOGGER {

public:
  vector<string> LOG{};

  virtual void WRITE() = 0;

  ofstream LOG_FILE;
  void WRITE_TO_FILE() {
    LOG_FILE.open(OUTPUT_FILE_PATH);
    if (LOG_FILE.is_open()) {
      for (const auto &line : LOG) {
        LOG_FILE << line << "\n";
      }
      LOG_FILE.close();

    } else {
      cerr << "Log file not open";
    }
  }

protected:
  string OUTPUT_FILE_PATH = "./";
};

class PPU_LOGGER : public LOGGER {
public:
  PPU_LOGGER(PPU_2C02 *ppu);
  PPU_2C02 *PPU = nullptr;
  void WRITE() override;
};

class CPU_LOGGER : public LOGGER {
public:
  CPU_LOGGER(CPU_6502 *cpu);
  CPU_6502 *CPU = nullptr;
  void WRITE() override;
};
