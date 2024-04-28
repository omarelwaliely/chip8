#include <SDL2/SDL.h>
#include <emulator/emulator.h>

const int WIDTH = 800, HEIGHT = 600;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("No File specified, EXITING\n");
        return -1;
    }
    Emulator em;
    startProgram(&em, argv[1]);
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL)
    {
        printf("Could not create window %s \n", SDL_GetError());
    }
    SDL_Event windowEvent;
    while (1)
    {
        if (SDL_PollEvent(&windowEvent))
        {
            if (SDL_QUIT == windowEvent.type)
            {
                break;
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}