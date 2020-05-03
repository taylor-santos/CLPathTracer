#ifndef OBJECT_H
#define OBJECT_H

#include <CL/cl_gl.h>
#include "vector3.h"

typedef struct Object Object;

struct __attribute__ ((packed)) Object {
    Vector3 position;
    enum {
        OBJ_SPHERE
    } type;
    union {
        struct {
            vec_t radius;
        } sphere;
    };
};

#define Object(position, type, object) ((Object){ position, type, object })

#endif//OBJECT_H
