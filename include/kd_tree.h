#ifndef KD_TREE_H
#define KD_TREE_H

#include "vector3.h"
#include "model.h"

typedef struct kd kd;
typedef struct kdnode kdnode;
typedef cl_int kd_index;

struct kd {
    kdnode *node_vec;
    Vector4 *vert_vec;
    cl_int3 *tri_vec;
};

typedef enum KD_AXIS {
    KD_X = 0, KD_Y = 1, KD_Z = 2
} KD_AXIS;

typedef enum KD_SIDE {
    KD_LEFT = 0,
    KD_RIGHT = 1,
    KD_DOWN = 2,
    KD_UP = 3,
    KD_BACK = 4,
    KD_FRONT = 5
} KD_SIDE;

struct __attribute__ ((packed)) kdnode {
    kd_index ropes[6];
    Vector3 min, max;
    enum {
        KD_SPLIT, KD_LEAF
    } type;
    union {
        struct {
            vec_t value;
            KD_AXIS axis;
            kd_index children[2];
        } split;
        struct {
            kd_index tris;
            kd_index tri_count;
        } leaf;
    };
};

kd
build_kd(Model *model);

void
delete_kd(kd tree);

#endif//KD_TREE_H
