#ifndef GLINIT_H
#define GLINIT_H

#include "CLInit.h"

typedef struct GL GL;
typedef struct GLData GLData;

struct GL {
    GLData *data;
    void (*GetWindowPos)(GL *this, int *x, int *y);
    void (*GetWindowSize)(GL *this, int *x, int *y);
    void (*RegisterKey)(GL *this, int key, GLFWkeyfun function);
    void (*Render)(GL *this);
    void (*Terminate)(GL *this);
};

GL
GLInit(const char *kernel_filename, const char *kernel_name);

#endif//GLINIT_H
