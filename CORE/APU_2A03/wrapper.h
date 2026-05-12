#include "Nes_Snd_Emu/nes_apu/Blip_Buffer.h"
#include "Nes_Snd_Emu/nes_apu/Nes_Apu.h"
#include <cstdint>
#include <vector>

using namespace std;

class BUS;

class APU_2A03 {

public:
  APU_2A03();

  void connect_bus(BUS *b);
  void RESET();

  void write_register(uint64_t cpu_cycle, uint16_t addr, uint8_t data);
  uint8_t read_status(uint64_t cpu_cycle);
  void END_FRAME(uint64_t cpu_cycle);
  vector<int16_t> TAKE_SAMPLES();

  const long SAMPLE_RATE = 44100;
  const long CPU_CLOCK_RATE = 1789773;

private:
  static int DMC_READ_CALLBACK(void *user_data, cpu_addr_t addr);

  cpu_time_t FRAME_RELATIVE_TIME(uint64_t cpu_cycle) const;
  int dmc_read(cpu_addr_t addr) const;

  BUS *B;
  uint64_t FRAME_START_CYCLE = 0;
  Blip_Buffer BUFFER;
  Nes_Apu APU;
};
