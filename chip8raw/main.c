#include "chip8.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

int main(){

    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Could not init!");
        return 1;
    }

    // Create the 98- x 480 window (original 64 x 32 scaled x15)
    SDL_Window* window = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 480, SDL_WINDOW_SHOWN);
    SDL_Renderer*  renderer = NULL;
    if (window != NULL) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == NULL) {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        }
    }
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    if (window == NULL || renderer == NULL || texture == NULL) {
        fprintf(stderr, "SDL setup failed: %s\n", SDL_GetError());
        if (texture != NULL) {
            SDL_DestroyTexture(texture);
        }
        if (renderer != NULL) {
            SDL_DestroyRenderer(renderer);
        }
        if (window != NULL) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
        return 1;
    }

    chip8 my_cpu;
    powerOn(&my_cpu);
    loadROM(&my_cpu, "Pong (alt).ch8");
    uint32_t pixel[2048];
    bool running = true;
    uint32_t last_timer_time = SDL_GetTicks();

    while (running) { //Essentially acts like a event checker. An event is any interaction with the OS. If the event is a closing action, the flag running turns flase and halts the game
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            // Maps the actual keybinds as events
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                uint8_t state = (event.type == SDL_KEYDOWN) ? 1 : 0;
                switch (event.key.keysym.sym) {
                    // Our modern interface does not use the same hexidecimal keypads of back them. We map a custom set of 16 buttons to the hexidecimal keypads our emulator expects
                    case SDLK_1: my_cpu.keypad[0x1] = state; break;
                    case SDLK_2: my_cpu.keypad[0x2] = state; break;
                    case SDLK_3: my_cpu.keypad[0x3] = state; break;
                    case SDLK_4: my_cpu.keypad[0xC] = state; break;
                    case SDLK_q: my_cpu.keypad[0x4] = state; break;
                    case SDLK_w: my_cpu.keypad[0x5] = state; break;
                    case SDLK_e: my_cpu.keypad[0x6] = state; break;
                    case SDLK_r: my_cpu.keypad[0xD] = state; break;
                    case SDLK_a: my_cpu.keypad[0x7] = state; break;
                    case SDLK_s: my_cpu.keypad[0x8] = state; break;
                    case SDLK_d: my_cpu.keypad[0x9] = state; break;
                    case SDLK_f: my_cpu.keypad[0xE] = state; break;
                    case SDLK_z: my_cpu.keypad[0xA] = state; break;
                    case SDLK_x: my_cpu.keypad[0x0] = state; break;
                    case SDLK_c: my_cpu.keypad[0xB] = state; break;
                    case SDLK_v: my_cpu.keypad[0xF] = state; break;
                }
            }
        }
        
        cycle(&my_cpu);

        uint32_t current_time = SDL_GetTicks();  //This manages the tick speed by using real life milliseconds. It checks if 16ms has passed then decriments the timers.
        if (current_time - last_timer_time >= 16) { // ~60Hz (1000ms / 60)
            if (my_cpu.delaytimer > 0) my_cpu.delaytimer--;
            if (my_cpu.soundtimer > 0) my_cpu.soundtimer--;
            last_timer_time = current_time;
        }

        for (int i = 0; i < 2048; ++i) {
            // Translate 1 to White, 0 to Black
            pixel[i] = (my_cpu.display[i] == 1) ? 0xFFFFFFFF : 0xFF000000;
        }
        // Update texture and render to screen
        SDL_UpdateTexture(texture, NULL, pixel, 64 * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // 4. Throttle Execution Speed
        SDL_Delay(2); // ~500 instructions per second
    }

    // Clean up memory
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}