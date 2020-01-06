#include <stdio.h>
#include <math.h>
#include "GLsetup.h"
#include "CLsetup.h"

int
main(int argc, char **argv) {
    int global_size = 64;
    int num_outputs = 8;
    int local_size = global_size / num_outputs / 4;
    float input1[global_size];
    float input2[global_size];

    if (argc != 3) {
        printf("usage: %s shader_file kernel_name\n", argv[0]);
        return 1;
    }
    GLState GL = GLsetup();
    CLState CL = CLsetup(argv[1], argv[2]);

    for (int i = 0; i < global_size; i++) {
        input1[i] = (float)i;
        input2[i] = (float)i * (float)i;
    }
    int index1 = CL.add_buffer(&CL, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        global_size * sizeof(*input1), input1);
    CL.set_arg(&CL, 0, sizeof(cl_mem *), index1);
    int index2 = CL.add_buffer(&CL, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        global_size * sizeof(*input2), input2);
    CL.set_arg(&CL, 1, sizeof(cl_mem *), index2);
    CL.set_arg(&CL, 2, local_size * sizeof(float), -1);
    int index3 = CL
        .add_buffer(&CL, CL_MEM_WRITE_ONLY, num_outputs * sizeof(float), NULL);
    CL.set_arg(&CL, 3, sizeof(cl_mem *), index3);
    CL.execute(&CL, global_size, local_size);
    {
        float output[num_outputs];
        CL.get_output(&CL, index3, output);
        float sum = 0;
        for (int i = 0; i < num_outputs; i++) {
            printf("%f\n", output[i]);
            sum += output[i];
        }
        printf("Sum: %f\n", sum);
    }
    for (int i = 0; i < global_size; i++) {
        input1[i] = 0;
        input2[i] = 1;
    }
    CL.write_buffer(&CL, index2, input1);
    CL.execute(&CL, global_size, local_size);
    {
        float output[num_outputs];
        CL.get_output(&CL, index3, output);
        float sum = 0;
        for (int i = 0; i < num_outputs; i++) {
            printf("%f\n", output[i]);
            sum += output[i];
        }
        printf("Sum: %f\n", sum);
    }
    CL.terminate(&CL);
    GL.terminate(&GL);
    return 0;
}
