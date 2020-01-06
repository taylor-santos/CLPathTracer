#define CLAMP(x, a, b) (x < (a) ? (a) : x > (b) ? (b) : x)

#include "CLsetup.h"
#include "CL/cl.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct buf {
    cl_mem buffer;
    size_t size;
} buf;

struct CLData {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_program program;
    cl_command_queue queue;
    cl_kernel kernel;
    int num_buffers;
    buf *buffers;
    int num_args;
};

static cl_platform_id
query_platform(const cl_platform_id *platforms, cl_uint num_platforms) {
    printf("There %s %d platform%s available:\n",
        num_platforms == 1
        ? "is"
        : "are",
        num_platforms,
        num_platforms == 1
        ? ""
        : "s");
    for (unsigned int i = 0; i < num_platforms; i++) {
        size_t name_len;
        HANDLE_ERR(clGetPlatformInfo(platforms[i],
            CL_PLATFORM_NAME,
            0,
            NULL,
            &name_len));
        char name[name_len];
        HANDLE_ERR(clGetPlatformInfo(platforms[i],
            CL_PLATFORM_NAME,
            name_len,
            name,
            NULL));
        printf("\t%d) %s\n", i + 1, name);
    }
    if (num_platforms == 1) {
        return platforms[0];
    }
    long int index;
    do {
        printf("Select a platform (1-%d):\n", num_platforms);
        printf("> ");
        char buf[64];
        fgets(buf, 64, stdin);
        index = strtol(buf, NULL, 10);
        if (index < 1 || index > (long)num_platforms) {
            printf("Invalid input. ");
        }
    } while (index < 1 || index > (long)num_platforms);
    return platforms[index - 1];
}

static cl_platform_id
get_platform(void) {
    cl_uint num_platforms;
    HANDLE_ERR(clGetPlatformIDs(0, NULL, &num_platforms));
    cl_platform_id platforms[num_platforms];
    HANDLE_ERR(clGetPlatformIDs(num_platforms, platforms, NULL));
    return query_platform(platforms, num_platforms);
}

static cl_device_id
query_device(const cl_device_id *devices, cl_uint num_devices) {
    printf("There %s %d device%s available:\n",
        num_devices == 1
        ? "is"
        : "are",
        num_devices,
        num_devices == 1
        ? ""
        : "s");
    for (unsigned int i = 0; i < num_devices; i++) {
        size_t name_len;
        HANDLE_ERR(clGetDeviceInfo(devices[i],
            CL_DEVICE_NAME,
            0,
            NULL,
            &name_len));
        char name[name_len];
        HANDLE_ERR(clGetDeviceInfo(devices[i],
            CL_DEVICE_NAME,
            name_len,
            name,
            NULL));
        printf("\t%d) %s\n", i + 1, name);
    }
    if (num_devices == 1) {
        return devices[0];
    }
    long int index;
    do {
        printf("Select a device (1-%d):\n", num_devices);
        printf("> ");
        char buf[64];
        fgets(buf, 64, stdin);
        index = strtol(buf, NULL, 10);
        if (index < 1 || index > (long)num_devices) {
            printf("Invalid input. ");
        }
    } while (index < 1 || index > (long)num_devices);
    return devices[index - 1];
}

static cl_device_id
get_device(cl_platform_id platform) {
    cl_uint num_devices;
    HANDLE_ERR(clGetDeviceIDs(platform,
        CL_DEVICE_TYPE_ALL,
        0,
        NULL,
        &num_devices));
    cl_device_id devices[num_devices];
    HANDLE_ERR(clGetDeviceIDs(platform,
        CL_DEVICE_TYPE_ALL,
        num_devices,
        devices,
        NULL));
    return query_device(devices, num_devices);
}

static size_t
get_max_work_group(cl_device_id device) {
    size_t size;

    HANDLE_ERR(clGetDeviceInfo(device,
        CL_DEVICE_MAX_WORK_GROUP_SIZE,
        sizeof(size_t),
        &size,
        NULL));
    return size;
}

static FILE *
open_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        char *msg = "Unable to open shader file";
        char out[strlen(msg) + strlen(filename) + 4];
        sprintf(out, "%s \"%s\"", msg, filename);
        perror(out);
        exit(EXIT_FAILURE);
    }
    return file;
}

static void
close_file(FILE *file) {
    if (fclose(file)) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }
}

static size_t
file_length(FILE *file) {
    long int length, prev_pos;

    prev_pos = ftell(file);
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("fseek");
        exit(EXIT_FAILURE);
    }
    if ((length = ftell(file)) < 0) {
        perror("ftell");
        exit(EXIT_FAILURE);
    }
    if (fseek(file, prev_pos, SEEK_SET) != 0) {
        perror("fseek");
        exit(EXIT_FAILURE);
    }
    return length;
}

static void
read_file(char *buffer, size_t file_len, FILE *file) {
    buffer[file_len] = '\0';
    if (fread(buffer, 1, file_len, file) != file_len && ferror(file) != 0) {
        fprintf(stderr, "error reading from file\n");
        exit(EXIT_FAILURE);
    }
}

static cl_context
create_context(cl_device_id device) {
    cl_context context;
    cl_int err;

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    HANDLE_ERR(err);
    return context;
}

static cl_program
build_program(cl_context context, cl_device_id device, const char *filename) {
    FILE *file;
    size_t length;
    char *src;
    int err;
    cl_program program;

    file = open_file(filename);
    length = file_length(file);
    src = malloc(length + 1);
    if (src == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    read_file(src, length, file);
    close_file(file);
    program = clCreateProgramWithSource(context,
        1,
        (const char **)&src,
        &length,
        &err);
    HANDLE_ERR(err);
    free(src);
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0) {
        HANDLE_ERR(clGetProgramBuildInfo(program,
            device,
            CL_PROGRAM_BUILD_LOG,
            0,
            NULL,
            &length));
        char log[length];
        HANDLE_ERR(clGetProgramBuildInfo(program,
            device,
            CL_PROGRAM_BUILD_LOG,
            length,
            log,
            NULL));
        printf("%s\n", log);
        exit(EXIT_FAILURE);
    }
    return program;
}

static cl_command_queue
create_queue(cl_context context, cl_device_id device) {
    cl_command_queue queue;
    cl_int err;

    queue = clCreateCommandQueue(context, device, 0, &err);
    HANDLE_ERR(err);
    return queue;
}

static cl_kernel
create_kernel(cl_program program, const char *kernel_name) {
    cl_kernel kernel;
    cl_int err;

    kernel = clCreateKernel(program, kernel_name, &err);
    HANDLE_ERR(err);
    return kernel;
}

static cl_mem
create_buffer(cl_context context, cl_mem_flags flags, size_t size,
    void *data) {
    cl_mem buffer;
    cl_int err;

    buffer = clCreateBuffer(context, flags, size, data, &err);
    HANDLE_ERR(err);
    return buffer;
}

static void
set_arg(cl_kernel kernel, int index, size_t size, cl_mem *value_ptr) {
    cl_int err;

    err = clSetKernelArg(kernel, index, size, value_ptr);
    HANDLE_ERR(err);
}

static void
enqueue_kernel(cl_command_queue queue, cl_kernel kernel, size_t global_size,
    size_t local_size) {
    cl_int err;

    err = clEnqueueNDRangeKernel(queue,
        kernel,
        1,
        NULL,
        &global_size,
        &local_size,
        0,
        NULL,
        NULL);
    HANDLE_ERR(err);
}

static void *
enqueue_output(cl_command_queue queue, size_t size, cl_mem output_buffer,
    void *output) {
    cl_int err;

    err = clEnqueueReadBuffer(queue,
        output_buffer,
        CL_TRUE,
        0,
        size,
        output,
        0,
        NULL,
        NULL);
    HANDLE_ERR(err);
    return output;
}

static int
add_buffer_CLState(CLState *this, cl_mem_flags flags, size_t size,
    void *data) {
    buf *new_buffers;
    int buf_index;

    buf_index = this->data->num_buffers++;
    new_buffers = realloc(this->data->buffers,
        this->data->num_buffers * sizeof(*this->data->buffers));
    if (new_buffers == NULL) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    this->data->buffers = new_buffers;
    this->data->buffers[buf_index] = (buf){
        create_buffer(this->data->context, flags, size, data),
        size
    };
    return buf_index;
}

static void
write_buffer_CLState(CLState *this, int index, const void *data) {
    HANDLE_ERR(clEnqueueWriteBuffer(this->data->queue,
        this->data->buffers[index].buffer,
        CL_TRUE,
        0,
        this->data->buffers[index].size,
        data,
        0,
        NULL,
        NULL));
}

static void
set_arg_CLState(CLState *this, int arg_index, size_t size, int buffer_index) {
    if (buffer_index < 0) {
        set_arg(this->data->kernel, arg_index, size, NULL);
    } else {
        set_arg(this->data->kernel,
            arg_index,
            size,
            &this->data->buffers[buffer_index].buffer);
    }
}

static void
execute_CLState(CLState *this, int global_size, int local_size) {
    local_size = CLAMP(local_size,
        1,
        (int)get_max_work_group(this->data->device));
    enqueue_kernel(this->data->queue,
        this->data->kernel,
        global_size,
        local_size);
}

static void *
get_output_CLState(CLState *this, int buffer_index, void *output) {
    return enqueue_output(this->data->queue,
        this->data->buffers[buffer_index].size,
        this->data->buffers[buffer_index].buffer,
        output);
}

static void
terminate_CLState(CLState *this) {
    free(this->data);
}

CLState
CLsetup(const char *filename, const char *kernel_name) {
    CLData *data;

    data = malloc(sizeof(*data));
    if (data == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    data->platform = get_platform();
    data->device = get_device(data->platform);
    data->context = create_context(data->device);
    data->program = build_program(data->context, data->device, filename);
    data->queue = create_queue(data->context, data->device);
    data->kernel = create_kernel(data->program, kernel_name);
    data->num_buffers = 0;
    data->buffers = NULL;
    data->num_args = 0;
    return (CLState){
        data,
        add_buffer_CLState,
        write_buffer_CLState,
        set_arg_CLState,
        execute_CLState,
        get_output_CLState,
        terminate_CLState
    };
}
