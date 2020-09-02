#include <stdio.h>

#include "game.h"
#include "list.h"
#include "voxel.h"

#define KERNEL_FILENAME "kernels/voxel.cl"
#define KERNEL_NAME     "render"

int
main(int argc, char **argv) {
    Voxel *voxels = voxel_generate();
    CLArg *args   = new_list(sizeof(CLArg));
    list_append(args, (CLArg){voxels});
    GameInit(KERNEL_FILENAME, KERNEL_NAME, args);
    StartGameLoop();
    GameTerminate();
    return 0;
}
