#ifndef VOXEL_H
#define VOXEL_H

#include "stdint.h"
#include "vector.h"

#define VOXEL_DEPTH 6u

#pragma pack(push, 1)
typedef struct Voxel {
    enum { INTERNAL, LEAF } type;
    union {
        struct {
            uint32_t children[8];
        } internal;
        struct {
            cl_float4 color;
        } leaf;
    };
} Voxel;
#pragma pack(pop)

Voxel *
voxel_generate(void);

#endif // VOXEL_H
