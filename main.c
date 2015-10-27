#include "SDL.h"
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
    SDL_Window *window;

    SDL_Init( SDL_INIT_EVERYTHING );

    window = SDL_CreateWindow("Demo 2",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              800,
                              600,
                              SDL_WINDOW_OPENGL);

    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    bool quit = false;
    SDL_Event e;

    while( !quit ) {
        while( SDL_PollEvent( &e ) != 0 ) {
            if( e.type == SDL_QUIT ) {
                quit = true;
            }
        }
        ///TODO: Create render function
    }

    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
