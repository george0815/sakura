#include "main.h"
#include "../CORE/BUS/bus.h"
#include "../CORE/CART/cart.h"
#include "../CORE/LOGGER/logger.h"
#include "./render.h"
#include "save.h"
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <boost/program_options.hpp>
#include <boost/program_options/detail/parsers.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

using namespace std;
using namespace boost::program_options;
using namespace nlohmann;
using namespace SAVE_MANAGER;

string get_time() {
  auto now = chrono::system_clock::now();
  time_t now_c = chrono::system_clock::to_time_t(now);

  stringstream time_as_string;
  time_as_string << put_time(localtime(&now_c), "%Y-%m-%d %H:%M:%S");
  return time_as_string.str();
}

// Translates special non-printable keys from Terminal.GUI's format to SDL2's
// format
int convert_special_key(int keyval) {

  switch (keyval) {
  case 10: // ENTER
    return 13;
  case 1048576: // UP
    return 1073741906;
  case 1048577: // DOWN
    return 1073741905;
  case 1048578: // LEFT
    return 1073741904;
  case 1048579: // RIGHT
    return 1073741903;

  // F1 - F12
  case 1048588: // F1
    return 1073741882;
  case 1048589: // F2
    return 1073741883;
  case 1048590: // F3
    return 1073741884;
  case 1048591: // F4
    return 1073741885;
  case 1048592: // F5
    return 1073741886;
  case 1048593: // F6
    return 1073741887;
  case 1048594: // F7
    return 1073741888;
  case 1048595: // F8
    return 1073741889;
  case 1048596: // F9
    return 1073741890;
  case 1048597: // F10
    return 1073741891;
  case 1048598: // F11
    return 1073741892;
  case 1048599: // F12
    return 1073741893;

  default:
    return keyval;
  }
};

int main(int arc, char *argv[]) {

  typedef struct {
    SDL_KeyCode SAVE;
    SDL_KeyCode LOAD;
    SDL_KeyCode STOP;
    SDL_KeyCode A;
    SDL_KeyCode B;
    SDL_KeyCode START;
    SDL_KeyCode SELECT;
    SDL_KeyCode UP;
    SDL_KeyCode DOWN;
    SDL_KeyCode LEFT;
    SDL_KeyCode RIGHT;
  } Controls;
  typedef struct {
    SHADER shader;
    RESOLUTION resolution;
    string log_path;
    string sram_path;
    string state_path;
    Controls controls;
    bool logging;

  } Settings;

  BUS bus;
  CPU_6502 cpu;
  PPU_2C02 ppu;
  Settings settings;

  deque<int16_t> AUDIO_QUEUE;
  mutex AUDIO_MUTEX;
  SDL_AudioDeviceID AUDIO_DEVICE;

  string rom_path = "";

  bool pending_save_state = false;

  bool pending_load_state = false;

  int last_audio_cycle = 0;

  options_description desc("Allowed options");

  desc.add_options()("rom", value<string>(), "path to rom");

  variables_map vm;

  store(parse_command_line(arc, argv, desc), vm);
  notify(vm);

  rom_path = vm["rom"].as<string>();

  ifstream settings_file("./cfg.json");

  json settingsData = json::parse(settings_file);

  // Set settings
  settings.log_path = settingsData["LogPath"];
  settings.sram_path = settingsData["SramPath"];
  settings.state_path = settingsData["StatePath"];
  settings.logging = settingsData["DetailedLogging"];
  // settings.shader = settingsData["Shader"];
  // settings.resolution = settingsData["Resolution"];
  settings.controls.START = static_cast<SDL_KeyCode>(
      convert_special_key(settingsData["Controls"]["START"].get<int>()));
  settings.controls.SELECT = static_cast<SDL_KeyCode>(
      convert_special_key(settingsData["Controls"]["SELECT"].get<int>()));
  settings.controls.A = static_cast<SDL_KeyCode>(
      convert_special_key(settingsData["Controls"]["A"].get<int>()));
  settings.controls.B = static_cast<SDL_KeyCode>(
      convert_special_key(settingsData["Controls"]["B"].get<int>()));
  settings.controls.UP = static_cast<SDL_KeyCode>(
      convert_special_key(settingsData["Controls"]["UP"].get<int>()));
  settings.controls.DOWN = static_cast<SDL_KeyCode>(
      convert_special_key(settingsData["Controls"]["DOWN"].get<int>()));
  settings.controls.LEFT = static_cast<SDL_KeyCode>(
      convert_special_key(settingsData["Controls"]["LEFT"].get<int>()));
  settings.controls.RIGHT = static_cast<SDL_KeyCode>(
      convert_special_key(settingsData["Controls"]["RIGHT"].get<int>()));
  settings.controls.SAVE = static_cast<SDL_KeyCode>(
      convert_special_key(settingsData["Controls"]["SaveState"].get<int>()));
  settings.controls.LOAD = static_cast<SDL_KeyCode>(
      convert_special_key(settingsData["Controls"]["LoadState"].get<int>()));
  settings.controls.STOP = static_cast<SDL_KeyCode>(
      convert_special_key(settingsData["Controls"]["StopRom"].get<int>()));

  unordered_map<SDL_Keycode, BUS::CONTROLLER_BUTTON> keymap = {

      {settings.controls.START, BUS::CONTROLLER_BUTTON::START},
      {settings.controls.SELECT, BUS::CONTROLLER_BUTTON::SELECT},
      {settings.controls.A, BUS::CONTROLLER_BUTTON::A},
      {settings.controls.B, BUS::CONTROLLER_BUTTON::B},
      {settings.controls.UP, BUS::CONTROLLER_BUTTON::UP},
      {settings.controls.DOWN, BUS::CONTROLLER_BUTTON::DOWN},
      {settings.controls.LEFT, BUS::CONTROLLER_BUTTON::LEFT},
      {settings.controls.RIGHT, BUS::CONTROLLER_BUTTON::RIGHT},

  };

  if (!init_sdl()) {
    return -1;
  }

  SDL_AudioSpec desired = {};
  SDL_AudioSpec obtained = {};

  desired.freq = 44100;
  desired.format = AUDIO_S16SYS;
  desired.channels = 1;
  desired.samples = 1024;
  desired.callback = nullptr;

  AUDIO_DEVICE = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);

  if (!AUDIO_DEVICE) {
    return -1;
  }

  SDL_PauseAudioDevice(AUDIO_DEVICE, 0);

  const double TARGET_FRAME_MS = 1000.0 / 60.0988;

  CART cart;

  filesystem::path p = rom_path;
  filesystem::path romFile = p.filename();

  string sramFilePath =
      settings.sram_path + (string)romFile.replace_extension(".sav");
  string stateFilePath =
      settings.state_path + (string)romFile.replace_extension("");

  string mostRecentStateFilePath = stateFilePath;
  // cout << sramFilePath << stateFilePath << endl;

  int test = PARSE_FILE(rom_path, cart);

  bus.insert_cartridge(cart);
  LOAD_SRAM(sramFilePath, &bus);
  cpu.connect_bus(&bus);
  bus.connect_cpu(cpu);
  bus.connect_ppu(ppu);

  ppu.connect_cpu(&cpu);

  cpu.RESET_HANDLER();

  PPU_LOGGER *ppu_logger = new PPU_LOGGER(bus.PPU);
  CPU_LOGGER *cpu_logger = new CPU_LOGGER(bus.CPU);

  bool running = true;

  SDL_Event event;

  while (running) {

    uint64_t start = SDL_GetTicks64();

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
      if (event.type == SDL_KEYDOWN) {

        SDL_Keycode key = event.key.keysym.sym;

        // get KV pair
        auto map_KV_pair = keymap.find(key);

        //->second is map value, first is map key
        if (map_KV_pair != keymap.end()) {
          bus.SET_CONTROLLER_BUTTON(0, map_KV_pair->second, true);
        }

        // if save state button
        else if (key == settings.controls.SAVE) {
          mostRecentStateFilePath =
              stateFilePath + "_" + get_time() + ".sakura";
          // cout << mostRecentStateFilePath << endl;
          pending_save_state = true;
        }
        // if load state button
        else if (key == settings.controls.LOAD) {
          pending_load_state = true;
        }
        // if stop rom button
        else if (key == settings.controls.STOP) {
          running = false;
        }

      } else if (event.type == SDL_KEYUP) {
        SDL_Keycode key = event.key.keysym.sym;

        // get KV pair
        auto map_KV_pair = keymap.find(key);

        //->second is map value, first is map key
        if (map_KV_pair != keymap.end()) {
          bus.SET_CONTROLLER_BUTTON(0, map_KV_pair->second, false);
        }
      }
    }

    if (pending_load_state) {
      pending_load_state = false;

      SDL_ClearQueuedAudio(AUDIO_DEVICE);

      if (!LOAD_STATE(mostRecentStateFilePath, &bus)) {
        cerr << "Failed to load save state";
        running = false;
        continue;
      }

      SDL_ClearQueuedAudio(AUDIO_DEVICE);
      ppu.CLEAR_FRAME_FLAG();

      continue;
    }

    int guard = 0;

    uint64_t frame_start_cycle = cpu.TOTAL_CYCLES;

    while (!ppu.FRAME_COMPLETE() && guard < 1000000) {
      cpu.step();

      while (cpu.CYCLES > 0) {
        cpu.step();
      }

      guard++;
    }

    bus.END_AUDIO_FRAME(cpu.TOTAL_CYCLES);

    if (pending_save_state) {
      pending_save_state = false;
      SAVE_STATE(mostRecentStateFilePath, &bus);
    }

    last_audio_cycle = cpu.TOTAL_CYCLES;

    auto samples = bus.TAKE_AUDIO_SAMPLES();

    if (!samples.empty()) {
      SDL_QueueAudio(AUDIO_DEVICE, samples.data(),
                     samples.size() * sizeof(int16_t));
    }

    if (settings.logging) {
      ppu_logger->WRITE();
      cpu_logger->WRITE();
    }

    render_frame(ppu.FRAMEBUFFER_DATA());
    ppu.CLEAR_FRAME_FLAG();

    uint64_t elapsed = SDL_GetTicks64() - start;

    while (SDL_GetQueuedAudioSize(AUDIO_DEVICE) > TARGET_QUEUE_BYTES) {
      SDL_Delay(1);
    }
  }

  SAVE_SRAM(sramFilePath, &bus);

  if (settings.logging) {
    ppu_logger->WRITE_TO_FILE();
    cpu_logger->WRITE_TO_FILE();
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
}
