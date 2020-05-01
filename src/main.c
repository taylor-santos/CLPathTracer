#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "model.h"

#define KERNEL_FILENAME "src/kernel.cl"
#define KERNEL_NAME "render"

int
main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <OBJ file>\n", argv[0]);
        return 1;
    }
    if (LoadModel(argv[1])) {
        exit(EXIT_FAILURE);
    }
    GameInit(KERNEL_FILENAME, KERNEL_NAME);
    StartGameLoop();
    GameTerminate();
    return 0;
}
