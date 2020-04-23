#include <stdio.h>
#include "game.h"

#define KERNEL_FILENAME "src/kernel.cl"
#define KERNEL_NAME "render"

int
main(int argc, char **argv) {
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 1;
    }
    GameInit(KERNEL_FILENAME, KERNEL_NAME);
    StartGameLoop();
    GameTerminate();
    return 0;
}
