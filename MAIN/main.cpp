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
#include <cstdint>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

using namespace std;
using namespace boost::program_options;
using namespace nlohmann;
using namespace SAVE_MANAGER;

int main(int arc, char *argv[]) {

  typedef struct {
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
    Controls controls;

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
  // settings.shader = settingsData["Shader"];
  // settings.resolution = settingsData["Resolution"];
  settings.controls.START =
      static_cast<SDL_KeyCode>(settingsData["Controls"]["START"].get<int>());
  settings.controls.SELECT =
      static_cast<SDL_KeyCode>(settingsData["Controls"]["SELECT"].get<int>());
  settings.controls.A =
      static_cast<SDL_KeyCode>(settingsData["Controls"]["A"].get<int>());
  settings.controls.B =
      static_cast<SDL_KeyCode>(settingsData["Controls"]["B"].get<int>());
  settings.controls.UP =
      static_cast<SDL_KeyCode>(settingsData["Controls"]["UP"].get<int>());
  settings.controls.DOWN =
      static_cast<SDL_KeyCode>(settingsData["Controls"]["DOWN"].get<int>());
  settings.controls.LEFT =
      static_cast<SDL_KeyCode>(settingsData["Controls"]["LEFT"].get<int>());
  settings.controls.RIGHT =
      static_cast<SDL_KeyCode>(settingsData["Controls"]["RIGHT"].get<int>());

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

  int test = PARSE_FILE(rom_path, cart);

  bus.insert_cartridge(cart);
  LOAD_SRAM("TEST.sav", &bus);
  cpu.connect_bus(&bus);
  bus.connect_cpu(cpu);
  bus.connect_ppu(ppu);

  ppu.connect_cpu(&cpu);

  cpu.RESET_HANDLER();

  PPU_LOGGER *ppu_logger = new PPU_LOGGER(bus.PPU);
  CPU_LOGGER *cpu_logger = new CPU_LOGGER(bus.CPU);

  bool running = true;

  SDL_Event event;

  cout << "\nDONE" << endl;

  while (running) {

    uint64_t start = SDL_GetTicks64();

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
      if (event.type == SDL_KEYDOWN) {

        SDL_Keycode key = event.key.keysym.sym;
        cout << "KEY PRESSED: " << SDL_GetKeyName(key) << endl;

        // get KV pair
        auto map_KV_pair = keymap.find(key);

        //->second is map value, first is map key
        if (map_KV_pair != keymap.end()) {
          cout << "KEY: " << SDL_GetKeyName(map_KV_pair->first) << endl;
          cout << "KEY PRESSED: " << endl;
          bus.SET_CONTROLLER_BUTTON(0, map_KV_pair->second, true);
        }

      } else if (event.type == SDL_KEYUP) {
        SDL_Keycode key = event.key.keysym.sym;

        // get KV pair
        auto map_KV_pair = keymap.find(key);

        //->second is map value, first is map key
        if (map_KV_pair != keymap.end()) {
          cout << "KEY RELEASED: " << endl;
          bus.SET_CONTROLLER_BUTTON(0, map_KV_pair->second, false);
        }
        /*
        switch (event.key.keysym.sym) {
        case SDLK_a:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::A, false);
          break;
        case SDLK_b:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::B, false);
          break;
        case SDLK_s:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::SELECT, false);
          break;
        case SDLK_f:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::START, false);
          break;
        case SDLK_UP:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::UP, false);
          break;
        case SDLK_DOWN:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::DOWN, false);
          break;
        case SDLK_LEFT:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::LEFT, false);
          break;
        case SDLK_RIGHT:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::RIGHT, false);
          break;
        case SDLK_r: // temporary save ram button
          cout << "R PRESSED";
          SAVE_SRAM("TEST.sav", &bus);
          break;
        case SDLK_k: // temporary save state button
          cout << "K PRESSED";
          pending_save_state = true;
          break;
        case SDLK_l: // temporary load state button
          cout << "L PRESSED";
          pending_load_state = true;
          break;
        case SDLK_e: // temporary exit button
          cout << "L PRESSED";
          running = false;

          break;
        }*/
      }
    }

    if (pending_load_state) {
      pending_load_state = false;

      SDL_ClearQueuedAudio(AUDIO_DEVICE);

      if (!LOAD_STATE("TEST.state", &bus)) {
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
      SAVE_STATE("TEST.state", &bus);
    }

    last_audio_cycle = cpu.TOTAL_CYCLES;

    auto samples = bus.TAKE_AUDIO_SAMPLES();

    if (!samples.empty()) {
      SDL_QueueAudio(AUDIO_DEVICE, samples.data(),
                     samples.size() * sizeof(int16_t));
    }

    ppu_logger->WRITE();
    cpu_logger->WRITE();
    render_frame(ppu.FRAMEBUFFER_DATA());
    ppu.CLEAR_FRAME_FLAG();

    uint64_t elapsed = SDL_GetTicks64() - start;

    while (SDL_GetQueuedAudioSize(AUDIO_DEVICE) > TARGET_QUEUE_BYTES) {
      SDL_Delay(1);
    }
  }

  ppu_logger->WRITE_TO_FILE();
  cpu_logger->WRITE_TO_FILE();
  SDL_DestroyWindow(window);
  SDL_Quit();
}
