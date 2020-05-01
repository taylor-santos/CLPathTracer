#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl_gl.h>
#include <GL/gl3w.h>

#include "error.h"

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

cl_platform_id
CLGetPlatform(void) {
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

cl_device_id
CLGetDevice(cl_platform_id platform) {
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

cl_context
CLCreateContext(cl_platform_id platform, cl_device_id device) {
    cl_context context;
    cl_int err;

    #ifdef WIN32
    cl_context_properties props[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform,
        CL_GL_CONTEXT_KHR,
        (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR,
        (cl_context_properties)wglGetCurrentDC(),
        0
    };
    #elif __APPLE__
    CGLContextObj glContext = CGLGetCurrentContext();
    CGLShareGroupObj shareGroup = CGLGetShareGroup(glContext);

    cl_context_properties props[] =
    {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)shareGroup,
        0
    };
    #else
    cl_context_properties props[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform,
        CL_GL_CONTEXT_KHR,
        (cl_context_properties)glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR,
        (cl_context_properties)glXGetCurrentDisplay(),
        0
    };
    #endif

    context = clCreateContext(props, 1, &device, NULL, NULL, &err);
    HANDLE_ERR(err);
    return context;
}

cl_program
CLBuildProgram(const char *filename, cl_context context, cl_device_id device) {
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

cl_command_queue
CLCreateQueue(cl_context context, cl_device_id device) {
    cl_command_queue queue;
    cl_int err;

    queue = clCreateCommandQueue(context, device, 0, &err);
    HANDLE_ERR(err);
    return queue;
}

cl_kernel
CLCreateKernel(const char *kernel_name, cl_program program) {
    cl_kernel kernel;
    cl_int err;

    kernel = clCreateKernel(program, kernel_name, &err);
    HANDLE_ERR(err);
    return kernel;
}

void
CLEnqueueKernel(cl_uint dim,
    size_t *global_size,
    size_t *local_size,
    cl_command_queue queue,
    cl_kernel kernel) {

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
