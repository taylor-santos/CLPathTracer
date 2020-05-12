#include <math.h>
#include <stdio.h>

#include "kd_tree.h"
#include "vector.h"
#include "model.h"

static kdnode
new_leaf(Vector3 min, Vector3 max, kd_index tris, kd_index tri_count) {
    return (kdnode){
            {
                    -1, -1, -1, -1, -1, -1
            }, min, max, KD_LEAF, .leaf={
                    tris, tri_count
            }
    };
}

static kdnode
new_split(Vector3 min, Vector3 max, vec_t value, KD_AXIS axis) {
    return (kdnode){
            {
                    -1, -1, -1, -1, -1, -1
            }, min, max, KD_SPLIT, .split={
                    value, axis, { -1, -1 }
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
    new_ropes[2 * axis] = node_vec[index].split.children[0];
    //kd_index prev = ropes[2 * axis];
    //ropes[2 * axis] = node_vec[index].split.children[0];
    add_ropes(node_vec, node_vec[index].split.children[1], new_ropes);
    for (int i = 0; i < 6; i++) {
        new_ropes[i] = ropes[i];
    }
    new_ropes[2 * axis + 1] = node_vec[index].split.children[1];
    //ropes[2 * axis] = prev;
    //prev = ropes[2 * axis + 1];
    //ropes[2 * axis + 1] = node_vec[index].split.children[1];
    add_ropes(node_vec, node_vec[index].split.children[0], new_ropes);
    //ropes[2 * axis + 1] = prev;
}

static kd_index
build_tree(kd *tree,
        const Vector4 *verts,
        const cl_int3 *tris,
        Vector3 min,
        Vector3 max,
        int depth,
        size_t minTris) {
    kd_index index = vector_length(tree->node_vec);
    if (vector_length(tris) <= minTris || depth == 0) {
        kd_index tri_index = (kd_index)vector_length(tree->tri_vec);
        kd_index tri_count = (kd_index)vector_length(tris);
        vector_append(tree->node_vec,
                new_leaf(min, max, tri_index, tri_count));
        vector_concat(tree->tri_vec, tris);
        return index;
    }
    /*
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
     */
    int found = 0;
    double best_ent;
    cl_int3 *low_tris, *high_tris;
    Vector3 low_max, high_min;
    vec_t median;
    KD_AXIS axis;
    int type;
    for (KD_AXIS new_axis = KD_X; new_axis <= KD_Z; new_axis++) {
        if (max.s[new_axis] - min.s[new_axis] < 0.01) {
            continue;
        }
        vec_t *values = new_vector();
        size_t num_tris = vector_length(tris);
        vec_t vmin = max.s[new_axis], vmax = min.s[new_axis];
        for (size_t i = 0; i < num_tris; i++) {
            cl_int3 tri = tris[i];
            for (int j = 0; j < 3; j++) {
                vec_t v = verts[tri.s[j]].s[new_axis];
                if (v < vmin) {
                    vmin = v;
                }
                if (v > vmax) {
                    vmax = v;
                }
                if (min.s[new_axis] < v && v < max.s[new_axis]) {
                    vector_append(values, v);
                }
            }
        }
        size_t num_values = vector_length(values);
        if (num_values == 0) {
            kd_index tri_index = (kd_index)vector_length(tree->tri_vec);
            kd_index tri_count = (kd_index)vector_length(tris);
            vector_append(tree->node_vec,
                    new_leaf(min, max, tri_index, tri_count));
            vector_concat(tree->tri_vec, tris);
            return index;
        }
        qsort(values, num_values, sizeof(*values), cmp_VEC_TYPE);
        vec_t new_median = values[num_values / 2];
        delete_vector(values);
        cl_int3 *new_low_tris = new_vector(), *new_high_tris = new_vector();
        Vector3 new_low_max = max, new_high_min = min;
        new_low_max.s[new_axis] = new_high_min.s[new_axis] = new_median;
        Vector3 low_ext = vec_subtract(new_low_max, min),
                high_ext = vec_subtract(max, new_high_min);
        vec_scale(&low_ext, 0.5);
        vec_scale(&high_ext, 0.5);
        Vector3 low_center = vec_add(min, low_ext),
                high_center = vec_add(new_high_min, high_ext);
        for (size_t i = 0; i < num_tris; i++) {
            cl_int3 tri = tris[i];
            Vector3 A = verts[tri.s[0]], B = verts[tri.s[1]],
                    C = verts[tri.s[2]];
            if (intersect_tri_AABB(low_center, low_ext, A, B, C)) {
                vector_append(new_low_tris, tri);
            }
            if (intersect_tri_AABB(high_center, high_ext, A, B, C)) {
                vector_append(new_high_tris, tri);
            }
        }
        size_t l = vector_length(new_low_tris),
                h = vector_length(new_high_tris);
        vec_t p = (vec_t)l / (vec_t)(l + h);
        vec_t e = -p * (vec_t)log2((double)p) -
                (1 - p) * (vec_t)log2(1 - (double)p);
        if (!found || e > best_ent) {
            type = 0;
            found = 1;
            best_ent = e;
            low_tris = new_low_tris;
            high_tris = new_high_tris;
            median = new_median;
            axis = new_axis;
            low_max = new_low_max;
            high_min = new_high_min;
        }
        /*
        p = (vmin - min.s[new_axis]) / (max.s[new_axis] - min.s[new_axis]);
        if (0 < p && p < 1) {
            if (p > 0.7) {
                type = 1;
                best_ent = e;
                low_tris = new_vector();
                high_tris = new_vector();
                vector_concat(high_tris, tris);
                median = vmin;
                axis = new_axis;
                low_max = new_low_max;
                high_min = new_high_min;
                break;
            }
        }
        p = 1 - (vmax - min.s[new_axis]) / (max.s[new_axis] - min.s[new_axis]);
        if (0 < p && p < 1) {
            if (p > 0.7) {
                type = 2;
                best_ent = e;
                low_tris = new_vector();
                high_tris = new_vector();
                vector_concat(low_tris, tris);
                median = vmax;
                axis = new_axis;
                low_max = new_low_max;
                high_min = new_high_min;
                break;
            }
        }
         */
    }
    if (!found) {
        kd_index tri_index = (kd_index)vector_length(tree->tri_vec);
        kd_index tri_count = (kd_index)vector_length(tris);
        vector_append(tree->node_vec,
                new_leaf(min, max, tri_index, tri_count));
        vector_concat(tree->tri_vec, tris);
        return index;
    }
    vector_append(tree->node_vec, new_split(min, max, median, axis));
    kd_index low_index = build_tree(tree,
            verts,
            low_tris,
            min,
            low_max,
            depth - 1,
            minTris);
    kd_index high_index = build_tree(tree,
            verts,
            high_tris,
            high_min,
            max,
            depth - 1,
            minTris);
    delete_vector(low_tris);
    delete_vector(high_tris);
    tree->node_vec[index].split.children[0] = low_index;
    tree->node_vec[index].split.children[1] = high_index;

    return index;
}

kd
build_kd(Model *model) {
    kd tree = {
            new_vector(), Model_verts(model), new_vector()
    };
    build_tree(&tree,
            Model_verts(model),
            Model_tris(model),
            Model_min(model),
            Model_max(model),
            10,
            0);
    add_ropes(tree.node_vec, 0, (kd_index[6]){
            -1, -1, -1, -1, -1, -1
    });
    const char *path = Model_path(model);
    if (path) {
        size_t size = snprintf(NULL, 0, "%s.kd", path);
        char kdpath[size + 1];
        sprintf(kdpath, "%s.kd", path);
        FILE *file = fopen(kdpath, "wb");
        size_t node_len = vector_length(tree.node_vec);
        fwrite(&node_len, sizeof(node_len), 1, file);
        fwrite(tree.node_vec, sizeof(*tree.node_vec), node_len, file);
        size_t vert_len = vector_length(tree.vert_vec);
        fwrite(&vert_len, sizeof(vert_len), 1, file);
        fwrite(tree.vert_vec, sizeof(*tree.vert_vec), vert_len, file);
        size_t tri_len = vector_length(tree.tri_vec);
        fwrite(&tri_len, sizeof(tri_len), 1, file);
        fwrite(tree.tri_vec, sizeof(*tree.tri_vec), tri_len, file);
        fclose(file);
    }
    return tree;
}

int
parse_kd(const char *filename, kd *tree) {
    FILE *file = fopen(filename, "rb");
    size_t node_len;
    fread(&node_len, sizeof(node_len), 1, file);
    tree->node_vec = init_vector(node_len, sizeof(*tree->node_vec));
    fread(tree->node_vec, sizeof(*tree->node_vec), node_len, file);
    size_t vert_len;
    fread(&vert_len, sizeof(vert_len), 1, file);
    tree->vert_vec = init_vector(vert_len, sizeof(*tree->vert_vec));
    fread((Vector4 *)tree->vert_vec, sizeof(*tree->vert_vec), vert_len, file);
    size_t tri_len;
    fread(&tri_len, sizeof(tri_len), 1, file);
    tree->tri_vec = init_vector(tri_len, sizeof(*tree->tri_vec));
    fread(tree->tri_vec, sizeof(*tree->tri_vec), tri_len, file);
    return 1;
}

void
delete_kd(kd tree) {
    delete_vector(tree.node_vec);
    delete_vector(tree.tri_vec);
}
