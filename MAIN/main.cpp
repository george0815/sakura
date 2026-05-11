#include "main.h"
#include "../CORE/CART/cart.h"
#include "./render.h"

#include <SDL2/SDL_events.h>
#include <iostream>

using namespace std;

int main(int arc, char *argv[]) {

  if (!init_sdl()) {
    return -1;
  }

  CART cart;

  // int test = PARSE_FILE("smb.nes", cart);
  int test = PARSE_FILE("nestest.nes", cart);

  bus.insert_cartridge(cart);
  cpu.connect_bus(&bus);
  bus.connect_cpu(cpu);
  bus.connect_ppu(ppu);
  cpu.RESET_HANDLER();

  bool running = true;

  SDL_Event event;

  cout << "\nDONE" << endl;
  int counter = 0;

  while (running) {

    if (counter >= 1000000000) {
      break;
    }

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    while (!ppu.FRAME_COMPLETE()) {
      cpu.step();
      if (cpu.CYCLES > 0) {
        cpu.step();
      }

      if (counter >= 1000000000) {
        break;
      }
      counter++;
    }

    cout << "COUNTER:" << to_string(counter) << endl;

    ppu.CLEAR_FRAME_FLAG();

    render_frame(ppu.FRAMEBUFFER_DATA());
  }
}
