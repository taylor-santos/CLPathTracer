#include <stdio.h>
#include <math.h>
#include "GLsetup.h"
#include "CLsetup.h"

int
main(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: %s shader_file kernel_name\n", argv[0]);
        return 1;
    }
    GLSetup();
    CLSetup(argv[1], argv[2]);
    GLRender();
    CLTerminate();
    GLTerminate();
    return 0;
}
