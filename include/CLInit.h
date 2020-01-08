#ifndef CL_SETUP_H
#define CL_SETUP_H

#include <stddef.h>
#include <CL/cl.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

typedef struct CL CL;
typedef struct CLData CLData;

struct CL {
    CLData *data;
    void (*DeleteImage)(CL *this);
    void (*CreateImage)(CL *this, GLuint texture);
    void (*Execute)(CL *this, int width, int height);
    void (*Terminate)(CL *this);
};

CL
CLInit(const char *kernel_filename, const char *kernel_name);

#endif//CL_SETUP_H
