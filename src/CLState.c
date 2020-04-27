#include <CL/cl_gl.h>
#include <math.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "error.h"
#include "camera.h"
#include "CLHandler.h"
#include "object.h"

static struct {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_program program;
    cl_command_queue queue;
    cl_kernel kernel;
    cl_mem image;
    cl_mem matrix;
    cl_mem objects;
    cl_int objcount;
} State;

void
CLDeleteImage(void) {
    HANDLE_ERR(clReleaseMemObject(State.image));
}

void
CLCreateImage(GLuint texture) {
    cl_int err;

    State.image = clCreateFromGLTexture(State.context,
        CL_MEM_WRITE_ONLY,
        GL_TEXTURE_2D,
        0,
        texture,
        &err);
    HANDLE_ERR(err);
}

static void
update_image(cl_command_queue queue, cl_mem *image, cl_kernel kernel) {
    HANDLE_ERR(clEnqueueAcquireGLObjects(queue, 1, image, 0, 0, NULL));
    clSetKernelArg(kernel, 0, sizeof(*image), image);
}

void
CLSetCameraMatrix(Matrix matrix) {
    HANDLE_ERR(clEnqueueWriteBuffer(State.queue,
        State.matrix,
        CL_TRUE,
        0,
        sizeof(Matrix),
        &matrix,
        0,
        NULL,
        NULL));
}

static void
set_arg(cl_kernel kernel, int index, size_t size, void *data) {
    HANDLE_ERR(clSetKernelArg(kernel, index, size, data));
}

static void
resize_object_buffer(size_t size) {
    cl_int err;
    State.objects =
        clCreateBuffer(State.context, CL_MEM_READ_ONLY, size, NULL, &err);
    HANDLE_ERR(err);
    set_arg(State.kernel, 2, sizeof(cl_mem), &State.objects);
    State.objcount = size / sizeof(Object);
    set_arg(State.kernel, 3, sizeof(cl_int), &State.objcount);
}

void
CLSetObjects(Object *objects, size_t size) {
    if (size / sizeof(Object) != State.objcount) {
        resize_object_buffer(size);
    }
    if (size == 0) {
        return;
    }
    HANDLE_ERR(clEnqueueWriteBuffer(State.queue,
        State.objects,
        CL_TRUE,
        0,
        size,
        objects,
        0,
        NULL,
        NULL));
}

void
CLExecute(int width, int height) {
    glFinish();
    update_image(State.queue, &State.image, State.kernel);
    CLEnqueueKernel(2, (size_t[]){
        width,
        height
    }, NULL, State.queue, State.kernel);
    clFinish(State.queue);
    HANDLE_ERR(clEnqueueReleaseGLObjects(State.queue,
        1,
        &State.image,
        0,
        0,
        NULL));
}

static cl_mem
create_buffer(cl_context context, size_t size) {
    cl_mem buffer;
    cl_int err;

    buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, size, NULL, &err);
    HANDLE_ERR(err);
    return buffer;
}

void
CLTerminate(void) {
}

void
CLInit(const char *kernel_filename, const char *kernel_name) {
    State.platform = CLGetPlatform();
    State.device = CLGetDevice(State.platform);
    State.context = CLCreateContext(State.platform, State.device);
    State.program =
        CLBuildProgram(kernel_filename, State.context, State.device);
    State.queue = CLCreateQueue(State.context, State.device);
    State.kernel = CLCreateKernel(kernel_name, State.program);
    State.matrix = create_buffer(State.context, sizeof(Matrix));
    set_arg(State.kernel, 1, sizeof(cl_mem), &State.matrix);
    set_arg(State.kernel, 2, sizeof(cl_mem), &State.objects);
    set_arg(State.kernel, 3, sizeof(cl_int), &State.objcount);
}
