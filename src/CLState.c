#include <CL/cl_gl.h>
#include <math.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "error.h"
#include "camera.h"
#include "CLHandler.h"
#include "object.h"
#include "vector.h"
#include "model.h"

typedef struct KernelArg {
    size_t size;
    void *arg_ptr;
    unsigned int is_ptr: 1;
} KernelArg;

#define KernelArg(size, arg_ptr, is_ptr) ((KernelArg){ size, arg_ptr, is_ptr })

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
    cl_mem verts;
    cl_int vertcount;
    cl_mem tris;
    cl_int tricount;
    KernelArg *vec_args;
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
update_args(cl_kernel kernel) {
    size_t count = vector_size(State.vec_args) / sizeof(KernelArg);
    for (size_t i = 0; i < count; i++) {
        HANDLE_ERR(clSetKernelArg(kernel,
            i,
            State.vec_args[i].size,
            State.vec_args[i].arg_ptr));
    }
}

static void
resize_buffer(cl_mem *buffer, size_t size) {
    if (*buffer != 0) {
        HANDLE_ERR(clReleaseMemObject(*buffer));
    }
    if (size == 0) {
        *buffer = 0;
        return;
    }
    *buffer = CLCreateBuffer(State.context, size);
}

void
CLSetObjects(Object *vec_objects, size_t size) {
    if (size / sizeof(Object) != (size_t)State.objcount) {
        resize_buffer(&State.objects, size);
        State.objcount = size / sizeof(Object);
    }
    if (size == 0) {
        return;
    }
    HANDLE_ERR(clEnqueueWriteBuffer(State.queue,
        State.objects,
        CL_TRUE,
        0,
        size,
        vec_objects,
        0,
        NULL,
        NULL));
}

void
CLSetMeshes(Model **models) {
    size_t model_count = vector_length(models);
    if (model_count == 0) {
        return;
    }
    Model *model = models[0];
    Vector4 *verts = Model_verts(model);
    cl_int3 *tris = Model_tris(model);
    size_t vertcount = vector_length(verts), tricount = vector_length(tris);
    if (vertcount != (size_t)State.vertcount) {
        resize_buffer(&State.verts, vertcount * sizeof(*verts));
        State.vertcount = vertcount;
    }
    if (tricount != (size_t)State.tricount) {
        resize_buffer(&State.tris, tricount * sizeof(*tris));
        State.tricount = tricount;
    }
    HANDLE_ERR(clEnqueueWriteBuffer(State.queue,
        State.verts,
        CL_TRUE,
        0,
        vertcount * sizeof(*verts),
        verts,
        0,
        NULL,
        NULL));
    HANDLE_ERR(clEnqueueWriteBuffer(State.queue,
        State.tris,
        CL_TRUE,
        0,
        tricount * sizeof(*tris),
        tris,
        0,
        NULL,
        NULL));
}

void
CLExecute(int width, int height) {
    glFinish();
    update_image(State.queue, &State.image, State.kernel);
    update_args(State.kernel);
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
    State.matrix = CLCreateBuffer(State.context, sizeof(Matrix));
    State.vec_args = new_vector();
    vector_append(State.vec_args, KernelArg(
        sizeof(cl_mem), &State.image, 0
    ));
    vector_append(State.vec_args, KernelArg(
        sizeof(cl_mem), &State.matrix, 0
    ));
    vector_append(State.vec_args, KernelArg(
        sizeof(cl_mem), &State.objects, 0
    ));
    vector_append(State.vec_args, KernelArg(
        sizeof(cl_int), &State.objcount, 1
    ));
    vector_append(State.vec_args, KernelArg(
        sizeof(cl_mem), &State.verts, 0
    ));
    vector_append(State.vec_args, KernelArg(
        sizeof(cl_int), &State.vertcount, 1
    ));
    vector_append(State.vec_args, KernelArg(
        sizeof(cl_mem), &State.tris, 0
    ));
    vector_append(State.vec_args, KernelArg(
        sizeof(cl_int), &State.tricount, 1
    ));
}
