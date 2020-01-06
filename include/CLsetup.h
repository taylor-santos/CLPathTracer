#ifndef CL_SETUP_H
#define CL_SETUP_H

#include <stddef.h>
#include <CL/cl.h>

typedef struct CLState CLState;
typedef struct CLData CLData;

struct CLState {
    CLData *data;
    int (*add_buffer)(CLState *this, cl_mem_flags flags, size_t size,
        void *data);
    void (*write_buffer)(CLState *this, int index, const void *data);
    void (*set_arg)(CLState *this, int arg_index, size_t size,
        int buffer_index);
    void (*execute)(CLState *this, int global_size, int local_size);
    void *(*get_output)(CLState *this, int buffer_index, void *output);
    void (*terminate)(CLState *this);
};

CLState
CLsetup(const char *filename, const char *kernel_name);

#endif//CL_SETUP_H
