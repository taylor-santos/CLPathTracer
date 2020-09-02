#include <CL/cl_gl.h>
#include <math.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "CLState.h"
#include "error.h"
#include "camera.h"
#include "CLHandler.h"
#include "object.h"
#include "list.h"
#include "kd_tree.h"

typedef struct KernelArg {
    size_t size;
    void * arg_ptr;
} KernelArg;

#define KernelArg(size, arg_ptr) ((KernelArg){size, arg_ptr})

static struct {
    cl_platform_id   platform;
    cl_device_id     device;
    cl_context       context;
    cl_program       program;
    cl_command_queue queue;
    cl_kernel        kernel;
    cl_mem           image;
    cl_mem           matrix;
    cl_mem           objects;
    cl_int           objcount;
    kd               kd;
    cl_mem           verts;
    cl_mem           norms;
    cl_mem           tris;
    cl_mem           triIndices;
    cl_mem           kdtree;
    size_t           treesize;
    KernelArg *      vec_args;
    cl_mem *         arg_buffers;
} State;

void
CLDeleteImage(void) {
    HANDLE_ERR(clReleaseMemObject(State.image));
}

void
CLCreateImage(GLuint texture) {
    cl_int err;

    State.image = clCreateFromGLTexture(
        State.context,
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

long long unsigned int count = 0;

void
CLSetCameraMatrix(Matrix matrix) {
    count++;
    HANDLE_ERR(clEnqueueWriteBuffer(
        State.queue,
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
    size_t count = list_length(State.vec_args);
    for (size_t i = 0; i < count; i++) {
        HANDLE_ERR(clSetKernelArg(
            kernel,
            i,
            State.vec_args[i].size,
            State.vec_args[i].arg_ptr));
    }
}

static void
resize_buffer(cl_mem *buffer, size_t size) {
    if (*buffer != 0) { HANDLE_ERR(clReleaseMemObject(*buffer)); }
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
    if (size == 0) { return; }
    HANDLE_ERR(clEnqueueWriteBuffer(
        State.queue,
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
CLSetMeshes(kd *models) {
    size_t model_count = list_length(models);
    if (model_count == 0) { return; }
    State.kd = models[0];
    {
        Vector4 *verts    = State.kd.vert_vec;
        size_t   vertSize = list_size(verts);
        resize_buffer(&State.verts, vertSize);
        HANDLE_ERR(clEnqueueWriteBuffer(
            State.queue,
            State.verts,
            CL_TRUE,
            0,
            vertSize,
            verts,
            0,
            NULL,
            NULL));
    }
    {
        Vector4 *norms    = State.kd.norm_vec;
        size_t   normSize = list_size(norms);
        if (normSize > 0) {
            resize_buffer(&State.norms, normSize);
            HANDLE_ERR(clEnqueueWriteBuffer(
                State.queue,
                State.norms,
                CL_TRUE,
                0,
                normSize,
                norms,
                0,
                NULL,
                NULL));
        }
    }
    {
        cl_int3 *tris    = State.kd.tri_vec;
        size_t   triSize = list_size(tris);
        resize_buffer(&State.tris, triSize);
        HANDLE_ERR(clEnqueueWriteBuffer(
            State.queue,
            State.tris,
            CL_TRUE,
            0,
            triSize,
            tris,
            0,
            NULL,
            NULL));
    }
    {
        int *  triIndices     = State.kd.tri_indices;
        size_t triIndicesSize = list_size(triIndices);
        resize_buffer(&State.triIndices, triIndicesSize);
        HANDLE_ERR(clEnqueueWriteBuffer(
            State.queue,
            State.triIndices,
            CL_TRUE,
            0,
            triIndicesSize,
            triIndices,
            0,
            NULL,
            NULL));
    }
    {
        size_t treesize = list_size(State.kd.node_vec);
        resize_buffer(&State.kdtree, treesize);
        HANDLE_ERR(clEnqueueWriteBuffer(
            State.queue,
            State.kdtree,
            CL_TRUE,
            0,
            treesize,
            State.kd.node_vec,
            0,
            NULL,
            NULL));
    }
}

void
CLExecute(int width, int height) {
    glFinish();
    update_image(State.queue, &State.image, State.kernel);
    update_args(State.kernel);
    CLEnqueueKernel(
        2,
        (size_t[]){width, height},
        NULL,
        State.queue,
        State.kernel);
    clFinish(State.queue);
    HANDLE_ERR(
        clEnqueueReleaseGLObjects(State.queue, 1, &State.image, 0, 0, NULL));
}

void
CLTerminate(void) {
    delete_kd(State.kd);
    delete_list(State.vec_args);
}

void
CLInit(const char *kernel_filename, const char *kernel_name, CLArg *args) {
    State.platform = CLGetPlatform();
    State.device   = CLGetDevice(State.platform);
    State.context  = CLCreateContext(State.platform, State.device);
    State.program =
        CLBuildProgram(kernel_filename, State.context, State.device);
    State.queue    = CLCreateQueue(State.context, State.device);
    State.kernel   = CLCreateKernel(kernel_name, State.program);
    State.matrix   = CLCreateBuffer(State.context, sizeof(Matrix));
    State.vec_args = new_list(9 * sizeof(*State.vec_args));
    list_append(State.vec_args, KernelArg(sizeof(cl_mem), &State.image));
    list_append(State.vec_args, KernelArg(sizeof(cl_mem), &State.matrix));
    if (args) {
        size_t num_args   = list_length(args);
        State.arg_buffers = new_list(sizeof(cl_mem) * num_args);
        for (size_t i = 0; i < num_args; i++) {
            CLArg  arg           = args[i];
            size_t sz            = list_size(arg.data);
            State.arg_buffers[i] = CLCreateBuffer(State.context, sz);
            HANDLE_ERR(clEnqueueWriteBuffer(
                State.queue,
                State.arg_buffers[i],
                CL_TRUE,
                0,
                sz,
                arg.data,
                0,
                NULL,
                NULL));
            list_append(
                State.vec_args,
                KernelArg(sizeof(cl_mem), State.arg_buffers + i));
        }
    }
    // vector_append(State.vec_args, KernelArg(sizeof(cl_mem), &State.objects,
    // 0));
    // vector_append(
    //     State.vec_args,
    //     KernelArg(sizeof(cl_int), &State.objcount, 1));
    // vector_append(State.vec_args, KernelArg(sizeof(cl_mem), &State.verts,
    // 0));
    // vector_append(State.vec_args, KernelArg(sizeof(cl_mem),
    // &State.norms,  0));
    // vector_append(State.vec_args,
    // KernelArg(sizeof(cl_mem), &State.tris, 0));
    // vector_append(
    //     State.vec_args,
    //     KernelArg(sizeof(cl_mem), &State.triIndices, 0));
    // vector_append(State.vec_args, KernelArg(sizeof(cl_mem), &State.kdtree,
    // 0));
}
