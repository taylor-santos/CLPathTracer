#ifndef OBJECT_H
#define OBJECT_H

#include <CL/cl_gl.h>

typedef struct Object Object;

struct __attribute__ ((packed)) Object {
    cl_double3 position;
    enum {
        OBJ_SPHERE
    } type;
    union {
        struct {
            cl_double radius;
        } sphere;
    };
};

#endif//OBJECT_H
