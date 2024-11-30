#include <iostream>
#include <vector>
#include "CPU.h"

void draw_penis(GFX_Display &display) {
    // balls
    display.activate(0, 0);
    display.activate(0, 1);
    display.activate(0, 2);

    // shaft
    display.activate(1, 1);
    display.activate(2, 1);
    display.activate(3, 1);
    display.activate(4, 1);
}

int main(int argc, char** argv) {
    
    // asser tthat arguments are valid
    if (argc != 2) {
        std::cout << "USAGE: init <GAME_FILE>" << std::endl;
        return 1; 
    }

    // Manage window & display
    Window window("CHIP-8 Emulator", 320, 640);
    GFX_Display display(window.renderer);

    //Framerate
    const unsigned int FPS = 60;
    unsigned int expDT = 1000 / FPS;

    //CPU Management
    C8_CPU state(argv[1], &display);

    while (window.is_open()) {
        unsigned long long initLoop = SDL_GetTicks();
        window.poll_events();
        state.setKeys( window.keys_pressed() );

        state.cycle();

        if (state.clear()) {
            display.clear();
            state.reset_clear();
        }

        window.clear();
        display.render();
        SDL_RenderPresent(window.renderer);
        
        unsigned long long dt = SDL_GetTicks() - initLoop;
        if (dt < expDT) SDL_Delay(expDT - dt);
    }

    SDL_Quit( );
    return 0;
}