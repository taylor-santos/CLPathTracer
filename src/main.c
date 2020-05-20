#include <stdio.h>

#include "game.h"
#include "list.h"

#define KERNEL_FILENAME "src/kernel.cl"
#define KERNEL_NAME "render"

int
main(int argc, char **argv) {
    const char **models = new_list(((size_t)argc - 1) * sizeof(*models));
    for (int i = 1; i < argc; i++) {
        vector_append(models, argv[i]);
    }
    GameInit(KERNEL_FILENAME, KERNEL_NAME, models);
    delete_list(models);
    StartGameLoop();
    GameTerminate();
    return 0;
}
