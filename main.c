#include <SDL2/SDL.h>
#include <emulator/emulator.h>

const int WIDTH = 800, HEIGHT = 600;

int main(int argc, char *argv[])
{

    Emulator em;
    startEmulator(&em);
    return 0;
}