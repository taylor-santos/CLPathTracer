#ifndef GLINIT_H
#define GLINIT_H

#include "CLInit.h"

typedef struct GL GL;
typedef struct GLData GLData;

struct GL {
    GLData *data;
    void (*GetWindowPos)(GL *this, int *x, int *y);
    void (*GetWindowSize)(GL *this, int *x, int *y);
    void (*GetMousePos)(GL *this, double *x, double *y);
    void (*RegisterKey)(GL *this, int key, GLFWkeyfun function);
    void (*RegisterMouseFunction)(GL *this, GLFWcursorposfun function);
    void (*SetCameraMatrix)(GL *this, cl_float4 matrix[static 4]);
    int (*Render)(GL *this);
    void (*Terminate)(GL *this);
};

GL
GLInit(const char *kernel_filename, const char *kernel_name);

#endif//GLINIT_H
