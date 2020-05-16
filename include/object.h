#ifndef OBJECT_H
#define OBJECT_H

#include <CL/cl_gl.h>
#include "vector3.h"

typedef struct Object Object;

#pragma pack(push, 1)
struct Object {
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
#pragma pack(pop)

#define Object(position, type, object) ((Object){ position, type, object })

#endif//OBJECT_H
