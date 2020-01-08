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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static cl_platform_id platform;
static cl_device_id device;
static cl_context context;
static cl_program program;
static cl_command_queue queue;
static cl_kernel kernel;
static cl_mem image;
static cl_mem matrix_buff;
static Camera camera;
static Matrix camMatrix;

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

static void
get_platform(void) {
    cl_uint num_platforms;
    HANDLE_ERR(clGetPlatformIDs(0, NULL, &num_platforms));
    cl_platform_id platforms[num_platforms];
    HANDLE_ERR(clGetPlatformIDs(num_platforms, platforms, NULL));
    platform = query_platform(platforms, num_platforms);
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

static void
get_device(void) {
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
    device = query_device(devices, num_devices);
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

static void
create_context(void) {
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
}

static void
build_program(const char *filename) {
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
}

static void
create_queue(void) {
    cl_int err;

    queue = clCreateCommandQueue(context, device, 0, &err);
    HANDLE_ERR(err);
}

static void
create_kernel(const char *kernel_name) {
    cl_int err;

    kernel = clCreateKernel(program, kernel_name, &err);
    HANDLE_ERR(err);
}

static void
enqueue_kernel(cl_uint dim, size_t *global_size, size_t *local_size) {
    cl_int err;

    err = clEnqueueNDRangeKernel(queue,
        kernel,
        dim,
        NULL,
        global_size,
        local_size,
        0,
        NULL,
        NULL);
    HANDLE_ERR(err);
}

void
CLCreateImage(void) {
    cl_int err;
    clReleaseMemObject(image);
    image = clCreateFromGLTexture(context,
        CL_MEM_WRITE_ONLY,
        GL_TEXTURE_2D,
        0,
        GLGetTexture(),
        &err);
    HANDLE_ERR(err);
}

void
CLExecute(int width, int height) {
    cl_int err;

    glFinish();
    HANDLE_ERR(clEnqueueAcquireGLObjects(queue, 1, &image, 0, 0, NULL));
    clSetKernelArg(kernel, 0, sizeof(image), &image);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double time = (double)tv.tv_sec + tv.tv_usec / 1000000.0;
    camera.set_position(&camera, new_Vector3(5 * cos(time), 0, 0));
    Matrix M1 = camera.camera_transform(&camera), M2 = camera
        .projection_transform(&camera), M3 = camera
        .device_transform(&camera, width, height);
    Matrix M4 = M3.times(&M3, &M2);
    M4 = M4.times(&M4, &M1);
    camMatrix = M4.inverse(&M4, NULL);
    cl_float4 matrix[4];
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            matrix[y].s[x] = camMatrix.values[4 * y + x];
        }
    }
    clReleaseMemObject(matrix_buff);
    matrix_buff = clCreateBuffer(context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(matrix),
        matrix,
        &err);
    HANDLE_ERR(err);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &matrix_buff);
    enqueue_kernel(2, (size_t[]){
        width,
        height
    }, NULL);
    clFinish(queue);
    HANDLE_ERR(clEnqueueReleaseGLObjects(queue, 1, &image, 0, 0, NULL));
}

void
CLTerminate(void) {
    camera.delete(&camera);
}

void
CLInit(const char *filename, const char *kernel_name) {
    get_platform();
    get_device();
    create_context();
    build_program(filename);
    create_queue();
    create_kernel(kernel_name);
    CLCreateImage();
    camera = new_Camera(2, 10, M_PI / 2);
}
