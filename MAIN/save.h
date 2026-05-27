#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

using namespace std;

class BUS;

namespace SAVE_MANAGER {

bool LOAD_SRAM(const string path, BUS *bus);

bool LOAD_STATE(const string path, BUS *bus);

bool SAVE_SRAM(const string path, BUS *bus);

bool SAVE_STATE(const string path, BUS *bus);

class StateWriter {

public:
  explicit StateWriter(vector<uint8_t> &bytes) : data(bytes) {}

  template <typename T> void plain_data(const T &value) {
    static_assert(is_trivially_copyable_v<T>,
                  "StateWriter requires data that can be copied easy.");
    const auto *raw = reinterpret_cast<const uint8_t *>(&value);
    data.insert(data.end(), raw, raw + sizeof(T));
  }

  template <typename T, size_t N> void array(const array<T, N> &values) {
    static_assert(is_trivially_copyable_v<T>,
                  "StateWriter requires data that can be copied easy.");
    const auto *raw = reinterpret_cast<const uint8_t *>(values.data());
    data.insert(data.end(), raw, raw + sizeof(T) * N);
  }

  void bytes(const vector<uint8_t> &values) {
    const uint32_t size = static_cast<uint32_t>(values.size());
    plain_data(size);
    data.insert(data.end(), values.begin(), values.end());
  }

  vector<uint8_t> &data;
};

class StateReader {

public:
  explicit StateReader(const vector<uint8_t> &bytes) : data(bytes) {}

  template <typename T> bool plain_data(T &value) {
    static_assert(is_trivially_copyable_v<T>,
                  "plain data requires data that can be copied easily");
    if (!can_read(sizeof(T))) {
      ok_ = false;
      // cout << "FALSE" << endl;
      return false;
    }

    memcpy(&value, data.data() + offset, sizeof(T));
    offset += sizeof(T);
    // cout << "TRUE" << endl;
    return true;
  }

  template <typename T, size_t N> bool array(array<T, N> &values) {
    static_assert(is_trivially_copyable_v<T>,
                  "array requires data that can be copied easily");
    const size_t bytes = sizeof(T) * N;
    if (!can_read(bytes)) {
      ok_ = false;
      // cout << "FALSE" << endl;
      return false;
    }

    memcpy(values.data(), data.data() + offset, bytes);
    offset += bytes;
    // cout << "TRUE" << endl;
    return true;
  }

  bool bytes(vector<uint8_t> &values) {
    uint32_t size = 0;
    if (!plain_data(size) || !can_read(size)) {
      ok_ = false;
      // cout << "FALSE" << endl;
      return false;
    }

    values.assign(data.begin() + static_cast<ptrdiff_t>(offset),
                  data.begin() + static_cast<ptrdiff_t>(offset + size));

    offset += size;

    // cout << "TRUE" << endl;
    return true;
  }

  bool ok() const { return ok_ && offset <= data.size(); }
  bool eof() const { return offset == data.size(); }

  bool can_read(size_t bytes) const { return offset + bytes <= data.size(); }

  const vector<uint8_t> &data;
  size_t offset = 0;
  bool ok_ = true;
};

} // namespace SAVE_MANAGER
