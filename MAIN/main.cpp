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

  int test = PARSE_FILE("smb.nes", cart);
  // int test = PARSE_FILE("nestest.nes", cart);

  bus.insert_cartridge(cart);
  bus.connect_cpu(cpu);
  bus.connect_ppu(ppu);
  cpu.connect_bus(&bus);
  cpu.RESET_HANDLER();

  bool running = true;

  SDL_Event event;

  cout << "\nDONE" << endl;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running - false;
      }
    }

    do {
      bus.step();
    } while (!ppu.FRAME_COMPLETE());

    render_frame(ppu.FRAMEBUFFER_DATA());
  }
}
