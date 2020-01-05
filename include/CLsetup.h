#ifndef CL_SETUP_H
#define CL_SETUP_H

#include <stddef.h>

typedef struct arg_struct {
    int num_inputs;
    size_t *input_lengths;
    void **inputs;
    int num_empty;
    size_t *empty_lengths;
    int num_outputs;
    size_t *output_lengths;
} arg_struct;

void**
CLsetup(
        const char *filename,
        const char *kernel_name,
        size_t global_size,
        size_t local_size,
        arg_struct *args
);

#endif//CL_SETUP_H
