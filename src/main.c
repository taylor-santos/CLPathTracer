#include <stdio.h>
#include <math.h>
#include "GameInit.h"

int
main(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: %s shader_file kernel_name\n", argv[0]);
        return 1;
    }

    GameInit(argv[1], argv[2]);
    StartGameLoop();
    return 0;
}
