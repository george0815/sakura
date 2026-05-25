#include "../CPU_6502/core.h"
#include "logger.h"
#include <string>

using namespace std;

CPU_LOGGER::CPU_LOGGER(CPU_6502 *cpu) {
  CPU_LOGGER::CPU = cpu;
  OUTPUT_FILE_PATH = "./cpu_log.txt";
}

void CPU_LOGGER::WRITE() {

  LOG.push_back("TOTAL_CYCLES: " + to_string(CPU->TOTAL_CYCLES));
  LOG.push_back("CYCLES: " + to_string(CPU->CYCLES));
  LOG.push_back("A: " + to_string(CPU->A));
  LOG.push_back("X: " + to_string(CPU->X));
  LOG.push_back("Y: " + to_string(CPU->Y));
  LOG.push_back("SP: " + to_string(CPU->SP));
  LOG.push_back("PC: " + to_string(CPU->PC));
  LOG.push_back("STATUS_REGISTER: " + to_string(CPU->STATUS_REGISTER));
  LOG.push_back("PENDING_INTERRUPT: " + to_string(CPU->PENDING_INTERRUPT));
  LOG.push_back("PAGE_CROSSED: " + to_string(CPU->PAGE_CROSSED));

  LOG.push_back("\n");
}
