#include "CLInit.h"
#include "CL/cl.h"
#include "error.h"
#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl_gl.h>
#include <sys/time.h>
#include <math.h>
#include <GLFW/glfw3.h>
#include <GL/gl3w.h>
#include "GLInit.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct CLData {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_program program;
    cl_command_queue queue;
    cl_kernel kernel;
    cl_mem image;
    cl_mem matrix_buff;
    Camera camera;
    Matrix camMatrix;
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
        CL_DEVICE_TYPE_DEFAULT,
        0,
        NULL,
        &num_devices));
    cl_device_id devices[num_devices];
    HANDLE_ERR(clGetDeviceIDs(platform,
        CL_DEVICE_TYPE_DEFAULT,
        num_devices,
        devices,
        NULL));
    return query_device(devices, num_devices);
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
create_context(cl_platform_id platform, cl_device_id device) {
    cl_context context;
    cl_int err;

    cl_context_properties props[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform,
        CL_GL_CONTEXT_KHR,
        (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR,
        (cl_context_properties)wglGetCurrentDC(),
        0
    };
    context = clCreateContext(props, 1, &device, NULL, NULL, &err);
    HANDLE_ERR(err);
    return context;
}

static cl_program
build_program(const char *filename, cl_context context, cl_device_id device) {
    cl_program program;
    FILE *file;
    size_t length;
    char *src;
    int err;

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
create_kernel(const char *kernel_name, cl_program program) {
    cl_kernel kernel;
    cl_int err;

    kernel = clCreateKernel(program, kernel_name, &err);
    HANDLE_ERR(err);
    return kernel;
}

static void
enqueue_kernel(cl_uint dim, size_t *global_size, size_t *local_size,
    cl_command_queue queue, cl_kernel kernel) {

    HANDLE_ERR(clEnqueueNDRangeKernel(queue,
        kernel,
        dim,
        NULL,
        global_size,
        local_size,
        0,
        NULL,
        NULL));
}

static void
delete_image(CL *this) {
    HANDLE_ERR(clReleaseMemObject(this->data->image));
}

static void
create_image(CL *this, GLuint texture) {
    cl_int err;

    this->data->image = clCreateFromGLTexture(this->data->context,
        CL_MEM_WRITE_ONLY,
        GL_TEXTURE_2D,
        0,
        texture,
        &err);
    HANDLE_ERR(err);
}

static void
execute(CL *this, int width, int height) {
    struct timeval tv;
    double time;
    cl_float4 matrix[4];

    glFinish();
    HANDLE_ERR(clEnqueueAcquireGLObjects(this->data->queue,
        1,
        &this->data->image,
        0,
        0,
        NULL));
    clSetKernelArg(this->data->kernel,
        0,
        sizeof(this->data->image),
        &this->data->image);
    gettimeofday(&tv, NULL);
    time = (double)tv.tv_sec + tv.tv_usec / 1000000.0;
    this->data->camera
        .set_position(&this->data->camera, new_Vector3(5 * cos(time), 0, 0));
    Matrix M1 = this->data->camera.camera_transform(&this->data->camera);
    Matrix M2 = this->data->camera.projection_transform(&this->data->camera);
    Matrix M3 = this->data->camera
        .device_transform(&this->data->camera, width, height);
    Matrix M4 = M3.times(&M3, &M2);
    M4 = M4.times(&M4, &M1);
    this->data->camMatrix = M4.inverse(&M4, NULL);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            matrix[y].s[x] = this->data->camMatrix.values[4 * y + x];
        }
    }
    clEnqueueWriteBuffer(this->data->queue,
        this->data->matrix_buff,
        CL_TRUE,
        0,
        sizeof(matrix),
        matrix,
        0,
        NULL,
        NULL);
    enqueue_kernel(2, (size_t[]){
        width,
        height
    }, NULL, this->data->queue, this->data->kernel);
    clFinish(this->data->queue);
    HANDLE_ERR(clEnqueueReleaseGLObjects(this->data->queue,
        1,
        &this->data->image,
        0,
        0,
        NULL));
}

static cl_mem
create_matrix_buffer(cl_context context) {
    cl_mem buffer;
    cl_int err;

    buffer = clCreateBuffer(context,
        CL_MEM_READ_ONLY,
        4 * sizeof(cl_float4),
        NULL,
        &err);
    HANDLE_ERR(err);
    return buffer;
}

static void
set_arg(cl_kernel kernel, int index, size_t size, void *data) {
    HANDLE_ERR(clSetKernelArg(kernel, 1, size, data));
}

static void
terminate(CL *this) {
    this->data->camera.delete(&this->data->camera);
}

CL
CLInit(const char *kernel_filename, const char *kernel_name) {
    CLData *data;

    data = malloc(sizeof(*data));
    if (data == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    data->platform = get_platform();
    data->device = get_device(data->platform);
    data->context = create_context(data->platform, data->device);
    data->program = build_program(kernel_filename,
        data->context,
        data->device);
    data->queue = create_queue(data->context, data->device);
    data->kernel = create_kernel(kernel_name, data->program);
    data->matrix_buff = create_matrix_buffer(data->context);
    set_arg(data->kernel, 1, sizeof(cl_mem), &data->matrix_buff);
    data->camera = new_Camera(2, 10, M_PI / 2);
    return (CL){
        data,
        delete_image,
        create_image,
        execute,
        terminate
    };;
}
