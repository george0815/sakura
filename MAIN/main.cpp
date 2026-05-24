#include "main.h"
#include "../CORE/BUS/bus.h"
#include "../CORE/CART/cart.h"
#include "../CORE/LOGGER/logger.h"
#include "./render.h"
#include "save.h"
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <boost/program_options.hpp>
#include <boost/program_options/detail/parsers.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <cstdint>
#include <iostream>

using namespace std;
using namespace boost::program_options;
using namespace SAVE_MANAGER;

int main(int arc, char *argv[]) {

  BUS bus;
  CPU_6502 cpu;
  PPU_2C02 ppu;

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
  cout << "FGheionhtiowentoinweoq" << endl;
  cout << rom_path << endl;

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
  LOGGER *logger = new LOGGER(false);

  int test = PARSE_FILE(rom_path, cart);
  //  int test = PARSE_FILE("mmc3_clock_test.nes", cart);
  //  int test = PARSE_FILE("mmc3_details_test.nes", cart);

  // int test = PARSE_FILE("mmc3_a12_test.nes", cart);
  //  int test = PARSE_FILE("mmc3_scanline_test.nes", cart);
  //    int test = PARSE_FILE("nestest.nes", cart);

  bus.insert_cartridge(cart);
  LOAD_SRAM("TEST.sav", &bus);
  cpu.connect_bus(&bus);
  bus.connect_cpu(cpu);
  bus.connect_ppu(ppu);

  ppu.connect_cpu(&cpu);

  cpu.RESET_HANDLER();

  cpu.init_logger(logger);

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
        // cout << "KEY PRESSED: " << event.key.keysym.sym;
        switch (event.key.keysym.sym) {
        case SDLK_a:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::A, true);
          break;
        case SDLK_b:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::B, true);
          break;
        case SDLK_s:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::SELECT, true);
          break;
        case SDLK_f:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::START, true);
          break;
        case SDLK_UP:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::UP, true);
          break;
        case SDLK_DOWN:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::DOWN, true);
          break;
        case SDLK_LEFT:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::LEFT, true);
          break;
        case SDLK_RIGHT:
          bus.SET_CONTROLLER_BUTTON(0, BUS::CONTROLLER_BUTTON::RIGHT, true);
          break;
        }
      } else if (event.type == SDL_KEYUP) {
        // cout << "KEY RELEASED: " << event.key.keysym.sym;
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
        }
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

    render_frame(ppu.FRAMEBUFFER_DATA());
    ppu.CLEAR_FRAME_FLAG();

    uint64_t elapsed = SDL_GetTicks64() - start;

    while (SDL_GetQueuedAudioSize(AUDIO_DEVICE) > TARGET_QUEUE_BYTES) {
      SDL_Delay(1);
    }
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
}
