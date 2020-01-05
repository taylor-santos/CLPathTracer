#include <stdio.h>
#include "CLsetup.h"

int
main(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: %s shader_file kernel_name\n", argv[0]);
        return 1;
    }
    int global_size = 128;
    int num_outputs = 16;
    int local_size = global_size/num_outputs/4;
    float input1[global_size];
    float input2[global_size];
    for (int i = 0; i < global_size; i++) {
        input1[i] = (float)i;
        input2[i] = (float)i*(float)i;
    }
    arg_struct args = {
        .num_inputs = 2,
        .input_lengths = (size_t[]){ global_size*sizeof(float),
                                     global_size*sizeof(float) },
        .inputs = (void*[]){ input1, input2 },
        .num_empty = 1,
        .empty_lengths = (size_t[]) { local_size*sizeof(float) },
        .num_outputs = 1,
        .output_lengths = (size_t[]){ num_outputs*sizeof(float) }
    };
    float **outputs = (float**) CLsetup(argv[1],
                                        argv[2],
                                        global_size,
                                        local_size,
                                        &args);
    float sum = 0;
    for (int i = 0; i < num_outputs; i++) {
        printf("%f\n", outputs[0][i]);
        sum += outputs[0][i];
    }
    printf("Sum: %f\n", sum);
    return 0;
}
