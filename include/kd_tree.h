#ifndef KD_TREE_H
#define KD_TREE_H

#include "vector.h"

typedef struct kd     kd;
typedef struct kdnode kdnode;
typedef cl_int        kd_index;

struct kd {
    kdnode * node_vec;
    int *    tri_indices;
    Vector4 *vert_vec;
    Vector4 *norm_vec;
    cl_int3 *tri_vec;
};

typedef enum KD_AXIS { KD_X = 0, KD_Y = 1, KD_Z = 2 } KD_AXIS;

typedef enum KD_SIDE {
    KD_LEFT  = 0,
    KD_RIGHT = 1,
    KD_DOWN  = 2,
    KD_UP    = 3,
    KD_BACK  = 4,
    KD_FRONT = 5
} KD_SIDE;

#pragma pack(push, 1)
struct kdnode {
    Vector4 min, max;
    enum { KD_SPLIT, KD_LEAF } type;
    union {
        struct {
            vec_t    value;
            KD_AXIS  axis;
            kd_index children[2];
        } split;
        struct {
            kd_index tris;
            kd_index tri_count;
            kd_index ropes[6];
        } leaf;
    };
};
#pragma pack(pop)

kd
build_kd(cl_int3 *tris, Vector3 *verts, Vector3 *norms, const char *path);

int
parse_kd(const char *filename, kd *tree);

void
delete_kd(kd tree);

#endif // KD_TREE_H
