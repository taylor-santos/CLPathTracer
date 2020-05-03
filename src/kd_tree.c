#include <math.h>
#include <stdio.h>

#include "kd_tree.h"
#include "vector.h"

static kdnode
new_leaf(Vector3 min, Vector3 max, kd_index tris, kd_index tri_count) {
    return (kdnode){
        {
            -1,
            -1,
            -1,
            -1,
            -1,
            -1
        },
        min,
        max,
        KD_LEAF, .leaf={
            tris,
            tri_count
        }
    };
}

static kdnode
new_split(Vector3 min, Vector3 max, vec_t value, KD_AXIS axis) {
    return (kdnode){
        {
            -1,
            -1,
            -1,
            -1,
            -1,
            -1
        },
        min,
        max,
        KD_SPLIT, .split={
            value,
            axis,
            -1,
            -1
        }
    };
}

static int
intersect_tri_AABB(Vector3 center,
    Vector3 extents,
    Vector3 A,
    Vector3 B,
    Vector3 C) {
    Vector3 offsetA = vec_subtract(A, center);
    Vector3 offsetB = vec_subtract(B, center);
    Vector3 offsetC = vec_subtract(C, center);

    Vector3 ba = vec_subtract(offsetB, offsetA);
    Vector3 cb = vec_subtract(offsetC, offsetB);

    vec_t x_ba_abs = fabs(vec_x(ba));
    vec_t y_ba_abs = fabs(vec_y(ba));
    vec_t z_ba_abs = fabs(vec_z(ba));

    {
        vec_t min = vec_z(ba) * vec_y(offsetA) - vec_y(ba) * vec_z(offsetA);
        vec_t max = vec_z(ba) * vec_y(offsetC) - vec_y(ba) * vec_z(offsetC);
        if (min > max) {
            vec_t temp = min;
            min = max;
            max = temp;
        }
        vec_t rad = z_ba_abs * vec_y(extents) + y_ba_abs * vec_z(extents);
        if (min > rad || max < -rad) {
            return 0;
        }
    }
    {
        vec_t min = -vec_z(ba) * vec_x(offsetA) + vec_x(ba) * vec_z(offsetA);
        vec_t max = -vec_z(ba) * vec_x(offsetC) + vec_x(ba) * vec_z(offsetC);
        if (min > max) {
            vec_t temp = min;
            min = max;
            max = temp;
        }
        vec_t rad = z_ba_abs * vec_x(extents) + x_ba_abs * vec_z(extents);
        if (min > rad || max < -rad) {
            return 0;
        }
    }
    {
        vec_t min = vec_y(ba) * vec_x(offsetB) - vec_x(ba) * vec_y(offsetB);
        vec_t max = vec_y(ba) * vec_x(offsetC) - vec_x(ba) * vec_y(offsetC);
        if (min > max) {
            vec_t temp = min;
            min = max;
            max = temp;
        }
        vec_t rad = y_ba_abs * vec_x(extents) + x_ba_abs * vec_y(extents);
        if (min > rad || max < -rad) {
            return 0;
        }
    }
    vec_t x_cb_abs = fabs(vec_x(cb));
    vec_t y_cb_abs = fabs(vec_y(cb));
    vec_t z_cb_abs = fabs(vec_z(cb));
    {
        vec_t min = vec_z(cb) * vec_y(offsetA) - vec_y(cb) * vec_z(offsetA),
            max = vec_z(cb) * vec_y(offsetC) - vec_y(cb) * vec_z(offsetC);
        if (min > max) {
            vec_t temp = min;
            min = max;
            max = temp;
        }
        vec_t rad = z_cb_abs * vec_y(extents) + y_cb_abs * vec_z(extents);
        if (min > rad || max < -rad) {
            return 0;
        }
    }
    {
        vec_t min = -vec_z(cb) * vec_x(offsetA) + vec_x(cb) * vec_z(offsetA),
            max = -vec_z(cb) * vec_x(offsetC) + vec_x(cb) * vec_z(offsetC);
        if (min > max) {
            vec_t temp = min;
            min = max;
            max = temp;
        }
        vec_t rad = z_cb_abs * vec_x(extents) + x_cb_abs * vec_z(extents);
        if (min > rad || max < -rad) {
            return 0;
        }
    }
    {
        vec_t min = vec_y(cb) * vec_x(offsetA) - vec_x(cb) * vec_y(offsetA),
            max = vec_y(cb) * vec_x(offsetB) - vec_x(cb) * vec_y(offsetB);
        if (min > max) {
            vec_t temp = min;
            min = max;
            max = temp;
        }
        vec_t rad = y_cb_abs * vec_x(extents) + x_cb_abs * vec_y(extents);
        if (min > rad || max < -rad) {
            return 0;
        }
    }
    Vector3 ac = vec_subtract(offsetA, offsetC);
    vec_t x_ac_abs = fabs(vec_x(ac));
    vec_t y_ac_abs = fabs(vec_y(ac));
    vec_t z_ac_abs = fabs(vec_z(ac));
    {
        vec_t min = vec_z(ac) * vec_y(offsetA) - vec_y(ac) * vec_z(offsetA),
            max = vec_z(ac) * vec_y(offsetB) - vec_y(ac) * vec_z(offsetB);
        if (min > max) {
            vec_t temp = min;
            min = max;
            max = temp;
        }
        vec_t rad = z_ac_abs * vec_y(extents) + y_ac_abs * vec_z(extents);
        if (min > rad || max < -rad) {
            return 0;
        }
    }
    {
        vec_t min = -vec_z(ac) * vec_x(offsetA) + vec_x(ac) * vec_z(offsetA),
            max = -vec_z(ac) * vec_x(offsetB) + vec_x(ac) * vec_z(offsetB);
        if (min > max) {
            vec_t temp = min;
            min = max;
            max = temp;
        }
        vec_t rad = z_ac_abs * vec_x(extents) + x_ac_abs * vec_z(extents);
        if (min > rad || max < -rad) {
            return 0;
        }
    }
    {
        vec_t min = vec_y(ac) * vec_x(offsetB) - vec_x(ac) * vec_y(offsetB),
            max = vec_y(ac) * vec_x(offsetC) - vec_x(ac) * vec_y(offsetC);
        if (min > max) {
            vec_t temp = min;
            min = max;
            max = temp;
        }
        vec_t rad = y_ac_abs * vec_x(extents) + x_ac_abs * vec_y(extents);
        if (min > rad || max < -rad) {
            return 0;
        }
    }
    {
        Vector3 normal = vec_cross(ba, cb);
        Vector3 min, max;
        if (vec_x(normal) > 0) {
            vec_x(min) = -vec_x(extents) - vec_x(offsetA);
            vec_x(max) = vec_x(extents) - vec_x(offsetA);
        } else {
            vec_x(min) = vec_x(extents) - vec_x(offsetA);
            vec_x(max) = -vec_x(extents) - vec_x(offsetA);
        }
        if (vec_y(normal) > 0) {
            vec_y(min) = -vec_y(extents) - vec_y(offsetA);
            vec_y(max) = vec_y(extents) - vec_y(offsetA);
        } else {
            vec_y(min) = vec_y(extents) - vec_y(offsetA);
            vec_y(max) = -vec_y(extents) - vec_y(offsetA);
        }
        if (vec_z(normal) > 0) {
            vec_z(min) = -vec_z(extents) - vec_z(offsetA);
            vec_z(max) = vec_z(extents) - vec_z(offsetA);
        } else {
            vec_z(min) = vec_z(extents) - vec_z(offsetA);
            vec_z(max) = -vec_z(extents) - vec_z(offsetA);
        }
        if (vec_dot(normal, min) > 0) {
            return 0;
        }
        if (vec_dot(normal, max) < 0) {
            return 0;
        }
    }
    {
        Vector3 min = vec_min(vec_min(offsetA, offsetB), offsetC);
        Vector3 max = vec_max(vec_max(offsetA, offsetB), offsetC);
        if (vec_x(min) > vec_x(extents) || vec_x(max) < -vec_x(extents)) {
            return 0;
        }
        if (vec_y(min) > vec_y(extents) || vec_y(max) < -vec_y(extents)) {
            return 0;
        }
        if (vec_z(min) > vec_z(extents) || vec_z(max) < -vec_z(extents)) {
            return 0;
        }
    }
    return 1;
}

static int
cmp_VEC_TYPE(const void *a, const void *b) {
    vec_t fa = *(const vec_t *)a;
    vec_t fb = *(const vec_t *)b;
    return (fa > fb) - (fa < fb);
}

static void
add_ropes(kdnode *node_vec, kd_index index, const kd_index ropes[static 6]) {
    for (KD_SIDE side = 0; side < 6; side++) {
        node_vec[index].ropes[side] = ropes[side];
    }
    if (node_vec[index].type == KD_LEAF) {
        return;
    }
    KD_AXIS axis = node_vec[index].split.axis;
    kd_index new_ropes[6];
    for (int i = 0; i < 6; i++) {
        new_ropes[i] = ropes[i];
    }
    new_ropes[2 * axis] = node_vec[index].split.low_child;
    //kd_index prev = ropes[2 * axis];
    //ropes[2 * axis] = node_vec[index].split.low_child;
    add_ropes(node_vec, node_vec[index].split.high_child, new_ropes);
    for (int i = 0; i < 6; i++) {
        new_ropes[i] = ropes[i];
    }
    new_ropes[2 * axis + 1] = node_vec[index].split.high_child;
    //ropes[2 * axis] = prev;
    //prev = ropes[2 * axis + 1];
    //ropes[2 * axis + 1] = node_vec[index].split.high_child;
    add_ropes(node_vec, node_vec[index].split.low_child, new_ropes);
    //ropes[2 * axis + 1] = prev;
}

static kd_index
build_tree(kd *tree, Vector4 *verts, cl_int3 *tris, Vector3 min, Vector3 max,
    //KD_AXIS axis,
    int depth, size_t minTris) {
    kd_index index = vector_length(tree->node_vec);
    if (vector_length(tris) < minTris || depth == 0) {
        kd_index tri_index = (kd_index)vector_length(tree->tri_vec);
        kd_index tri_count = (kd_index)vector_length(tris);
        vector_append(tree->node_vec,
            new_leaf(min, max, tri_index, tri_count));
        vector_concat(tree->tri_vec, tris);
        return index;
    }
    Vector3 extents = vec_subtract(max, min);
    vec_t x = vec_x(extents), y = vec_y(extents), z = vec_z(extents);
    KD_AXIS axis;
    if (x >= y) {
        if (x >= z) {
            axis = KD_X;
        } else { // x < z
            axis = KD_Z;
        }
    } else { // x < y
        if (z >= y) {
            axis = KD_Z;
        } else { // z < y
            axis = KD_Y;
        }
    }
    cl_int3 *best_low_tris = NULL, *best_high_tris = NULL;
    double entropy = 0;
    int found = 0;
    KD_AXIS best_axis;
    vec_t best_median;
    Vector3 best_low_max;
    Vector3 best_high_min;
    //for (KD_AXIS axis = KD_X; axis <= KD_Z; axis++) {
    vec_t *values = new_vector();
    size_t num_tris = vector_length(tris);
    for (size_t i = 0; i < num_tris; i++) {
        cl_int3 tri = tris[i];
        vec_t avg = 0;
        for (int j = 0; j < 3; j++) {
            avg += verts[tri.s[j]].s[axis];
        }
        avg /= 3;
        if (min.s[axis] < avg && avg < max.s[axis]) {
            vector_append(values, avg);
        }
    }
    size_t num_values = vector_length(values);
    qsort(values, num_values, sizeof(*values), cmp_VEC_TYPE);
    vec_t median = values[num_values / 2];
    delete_vector(values);
    cl_int3 *low_tris = new_vector(), *high_tris = new_vector();
    Vector3 low_max = max, high_min = min;
    low_max.s[axis] = high_min.s[axis] = median;
    Vector3 low_ext = vec_subtract(low_max, min),
        high_ext = vec_subtract(max, high_min);
    vec_scale(&low_ext, 0.5);
    vec_scale(&high_ext, 0.5);
    Vector3 low_center = vec_add(min, low_ext),
        high_center = vec_add(high_min, high_ext);
    for (size_t i = 0; i < num_tris; i++) {
        cl_int3 tri = tris[i];
        Vector3 A = verts[tri.s[0]], B = verts[tri.s[1]], C = verts[tri.s[2]];
        if (intersect_tri_AABB(low_center, low_ext, A, B, C)) {
            vector_append(low_tris, tri);
        }
        if (intersect_tri_AABB(high_center, high_ext, A, B, C)) {
            vector_append(high_tris, tri);
        }
    }
    /*
        size_t low_tri_count = vector_length(low_tris),
            high_tri_count = vector_length(high_tris);
        double p =
            (double)low_tri_count / (double)(low_tri_count + high_tri_count);
        double new_entropy = -p * log2(p) - (1 - p) * log2(1 - p);
        if (!found || new_entropy > entropy) {
            delete_vector(best_low_tris);
            delete_vector(best_high_tris);
            found = 1;
            entropy = new_entropy;
            best_low_tris = low_tris;
            best_high_tris = high_tris;
            best_axis = axis;
            best_median = median;
            best_low_max = low_max;
            best_high_min = high_min;
        }
    }*/
    delete_vector(best_low_tris);
    delete_vector(best_high_tris);
    found = 1;
    best_low_tris = low_tris;
    best_high_tris = high_tris;
    best_axis = axis;
    best_median = median;
    best_low_max = low_max;
    best_high_min = high_min;

    size_t low_tri_count = vector_length(best_low_tris),
        high_tri_count = vector_length(best_high_tris);
    /*
    if (low_tri_count == 0 || high_tri_count == 0) {
        delete_vector(best_low_tris);
        delete_vector(best_high_tris);
        kd_index tri_index = (kd_index)vector_length(tree->tri_vec);
        kd_index tri_count = (kd_index)vector_length(tris);
        vector_append(tree->node_vec,
            new_leaf(min, max, tri_index, tri_count));
        vector_concat(tree->tri_vec, tris);
        return index;
    }
     */
    vector_append(tree->node_vec, new_split(min, max, best_median, best_axis));
    kd_index
        low_index = build_tree(tree, verts, best_low_tris, min, best_low_max,
        //(axis + 1) % 3,
        depth - 1, minTris);
    kd_index high_index =
        build_tree(tree, verts, best_high_tris, best_high_min, max,
            //(axis + 1) % 3,
            depth - 1, minTris);
    delete_vector(best_low_tris);
    delete_vector(best_high_tris);
    tree->node_vec[index].split.low_child = low_index;
    tree->node_vec[index].split.high_child = high_index;

    return index;
}

kd
build_kd(Model *model) {
    kd tree = {
        new_vector(),
        Model_verts(model),
        new_vector()
    };
    build_tree(&tree,
        Model_verts(model),
        Model_tris(model),
        Model_min(model),
        Model_max(model),
        15,
        5);
    add_ropes(tree.node_vec, 0, (kd_index[6]){
        -1,
        -1,
        -1,
        -1,
        -1,
        -1
    });
    return tree;
}

void
delete_kd(kd tree) {
    delete_vector(tree.node_vec);
    delete_vector(tree.tri_vec);
}
