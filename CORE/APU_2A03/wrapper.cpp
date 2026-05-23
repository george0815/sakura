#include "wrapper.h"
#include "../BUS/bus.h"
#include "Nes_Snd_Emu/nes_apu/Nes_Apu.h"
#include "Nes_Snd_Emu/nes_apu/apu_snapshot.h"
#include <cstdint>
#include <stdexcept>

using namespace std;
using namespace SAVE_MANAGER;

APU_2A03::APU_2A03() {
  if (const char *err = BUFFER.sample_rate(SAMPLE_RATE, 100)) {
    throw runtime_error(err);
  }

  BUFFER.clock_rate(CPU_CLOCK_RATE);
  BUFFER.bass_freq(20);
  APU.output(&BUFFER);
  APU.dmc_reader(&APU_2A03::DMC_READ_CALLBACK, this);
  APU.volume(0.7);
  RESET();
}

void APU_2A03::connect_bus(BUS *bus_ptr) { B = bus_ptr; }

void APU_2A03::RESET() {
  BUFFER.clear();
  APU.reset(false);
  FRAME_START_CYCLE = 0;
}

void APU_2A03::write_register(uint64_t cpu_cycle, uint16_t addr, uint8_t data) {
  APU.write_register(FRAME_RELATIVE_TIME(cpu_cycle), addr, data);
}

uint8_t APU_2A03::read_status(uint64_t cpu_cycle) {
  return APU.read_status(FRAME_RELATIVE_TIME(cpu_cycle));
}

void APU_2A03::END_FRAME(uint64_t cpu_cycle) {
  const cpu_time_t frame_cycles = FRAME_RELATIVE_TIME(cpu_cycle);
  APU.end_frame(frame_cycles);
  BUFFER.end_frame(frame_cycles);
  FRAME_START_CYCLE = cpu_cycle;
}

vector<int16_t> APU_2A03::TAKE_SAMPLES() {
  const long samples = BUFFER.samples_avail();
  vector<int16_t> pcm(samples);
  if (samples > 0) {
    BUFFER.read_samples(pcm.data(), samples);
  }
  return pcm;
}

int APU_2A03::DMC_READ_CALLBACK(void *user_data, cpu_addr_t addr) {
  return static_cast<APU_2A03 *>(user_data)->dmc_read(addr);
}

cpu_time_t APU_2A03::FRAME_RELATIVE_TIME(uint64_t cpu_cycle) const {
  return (cpu_cycle - FRAME_START_CYCLE);
}

int APU_2A03::dmc_read(cpu_addr_t addr) const { return B ? B->read(addr) : 0; }

void APU_2A03::save_state(StateWriter &writer) const {
  writer.plain_data(FRAME_START_CYCLE);
  apu_snapshot_t snapshot{};
  APU.save_snapshot(&snapshot);
  writer.plain_data(snapshot);
}

bool APU_2A03::load_state(StateReader &reader) {
  apu_snapshot_t snapshot{};
  if (!reader.plain_data(FRAME_START_CYCLE) || !reader.plain_data(snapshot)) {
    return false;
  }

  BUFFER.clear();
  APU.load_snapshot(snapshot);
  return true;
}
