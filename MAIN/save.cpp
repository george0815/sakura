#include "./save.h"
#include "../CORE/BUS/bus.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <vector>

bool SAVE_MANAGER::SAVE_SRAM(const string path, BUS *bus) {

  if (!bus->BATTERY_BACKED) {
    // cout << "NO  SRAM";
    return true;
  }

  const auto &data = bus->PRG_RAM;

  filesystem::path p = path;

  filesystem::create_directories(p.parent_path());

  ofstream file(path, ios::binary | ios::trunc);

  file.write(reinterpret_cast<const char *>(data.data()),
             static_cast<streamsize>(data.size()));

  if (!file.good()) {
    // cout << "SOMETYHING WRONG WITH FILE";
    return false;
  }

  // cout << "RAM SAVED";
  return true;
}

bool SAVE_MANAGER::LOAD_SRAM(const string path, BUS *bus) {

  if (!bus->BATTERY_BACKED) {
    return true;
  }

  if (!filesystem::exists(path)) {
    return true;
  }

  ifstream file(path, ios::binary);
  if (!file.is_open()) {
    return false;
  }

  vector<uint8_t> data;

  file.seekg(0, ios::end);
  const streamsize size = file.tellg();
  file.seekg(0, ios::beg);

  data.resize(static_cast<size_t>(size));

  file.read(reinterpret_cast<char *>(data.data()), size);

  if (!file.good() && !file.eof()) {
    // cout << "FILE AINT GOOD";
    return false;
  }
  if (!bus->LOAD_BATTERY_BACKED_SRAM(data)) {
    // cout << "SOMETHIGN  WENT WRONG LOADING DATA";
    return false;
  }
  // cout << "SRAM LOADED";
  return true;
}

bool SAVE_MANAGER::SAVE_STATE(const string path, BUS *bus) {

  vector<uint8_t> bytes;
  StateWriter writer(bytes);
  writer.plain_data(0x52595354);
  writer.plain_data(1);
  bus->CPU->save_state(writer);
  bus->PPU->save_state(writer);
  bus->save_state(writer);

  filesystem::path p = path;

  filesystem::create_directories(p.parent_path());
  ofstream file(path, ios::binary | ios::trunc);

  file.write(reinterpret_cast<const char *>(bytes.data()),
             static_cast<streamsize>(bytes.size()));

  if (!file.good()) {
    // cout << "SOMETYHING WRONG WITH FILE";
    return false;
  }

  // cout << "STATE SAVED" << endl;

  return true;
}

bool SAVE_MANAGER::LOAD_STATE(const string path, BUS *bus) {

  if (!filesystem::exists(path)) {
    return true;
  }

  ifstream file(path, ios::binary);
  if (!file.is_open()) {
    return false;
  }

  vector<uint8_t> data;

  file.seekg(0, ios::end);
  const streamsize size = file.tellg();
  file.seekg(0, ios::beg);

  data.resize(static_cast<size_t>(size));

  file.read(reinterpret_cast<char *>(data.data()), size);

  if (!file.good() && !file.eof()) {
    // cout << "FILE AINT GOOD";
    return false;
  }

  StateReader reader(data);

  uint32_t magic = 0;
  uint32_t version = 0;

  if (

      !reader.plain_data(magic) || !reader.plain_data(version) ||
      magic != 0x52595354 || version != 1 || !bus->CPU->load_state(reader) ||
      !bus->PPU->load_state(reader) || !bus->load_state(reader) ||
      !reader.ok() || !reader.eof()) {

    // cout << "SOMETHING FAILED HERE" << endl;
    return false;
  }

  // cout << "STATE LOADED" << endl;

  return true;
}
