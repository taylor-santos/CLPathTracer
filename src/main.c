#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "vector.h"

#define KERNEL_FILENAME "src/kernel.cl"
#define KERNEL_NAME "render"

int
main(int argc, char **argv) {
    const char **models = new_vector();
    for (int i = 1; i < argc; i++) {
        vector_append(models, argv[i]);
    }
    GameInit(KERNEL_FILENAME, KERNEL_NAME, models);
    StartGameLoop();
    GameTerminate();
    return 0;
}
