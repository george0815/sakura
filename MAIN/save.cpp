#include "./save.h"
#include "../CORE/BUS/bus.h"
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <vector>

bool SAVE_MANAGER::SAVE_SRAM(const string path, BUS *bus) {

  if (!bus->BATTERY_BACKED) {
    cout << "NO  SRAM";
    return true;
  }

  const auto &data = bus->PRG_RAM;

  ofstream file(path, ios::binary | ios::trunc);

  file.write(reinterpret_cast<const char *>(data.data()),
             static_cast<streamsize>(data.size()));

  if (!file.good()) {
    cout << "SOMETYHING WRONG WITH FILE";
    return false;
  }

  cout << "RAM SAVED";
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
    cout << "FILE AINT GOOD";
    return false;
  }
  if (!bus->LOAD_BATTERY_BACKED_SRAM(data)) {
    cout << "SOMETHIGN  WENT WRONG LOADING DATA";
    return false;
  }
  cout << "SRAM LOADED";
  return true;
}
