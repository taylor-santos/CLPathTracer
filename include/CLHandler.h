#ifndef CLHANDLER_H
#define CLHANDLER_H

#include <CL/cl.h>

cl_platform_id
CLGetPlatform(void);
cl_device_id
CLGetDevice(cl_platform_id platform);
cl_context
CLCreateContext(cl_platform_id platform, cl_device_id device);
cl_program
CLBuildProgram(const char *filename, cl_context context, cl_device_id device);
cl_command_queue
CLCreateQueue(cl_context context, cl_device_id device);
cl_kernel
CLCreateKernel(const char *kernel_name, cl_program program);
cl_mem
CLCreateBuffer(cl_context context, size_t size);
void
CLEnqueueKernel(
    cl_uint          dim,
    size_t *         global_size,
    size_t *         local_size,
    cl_command_queue queue,
    cl_kernel        kernel);

#endif // CLHANDLER_H
