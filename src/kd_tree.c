#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "kd_tree.h"
#include "vector.h"

#define DEPTH 30

typedef struct triangle {
    int index;
    Vector3 V[3], center;
} triangle;

static int leafCount = 0;
static int leafTriCount = 0;

static kdnode
new_leaf(Vector3 min, Vector3 max, kd_index tris, kd_index tri_count) {
    leafCount++;
    leafTriCount += tri_count;
    return (kdnode){
            min, max, KD_LEAF, .leaf={
                    tris, tri_count, {
                            -1, -1, -1, -1, -1, -1
                    }
            }
    };
}

static kdnode
new_split(Vector3 min, Vector3 max, vec_t value, KD_AXIS axis) {
    return (kdnode){
            min, max, KD_SPLIT, .split={
                    value, axis, { -1, -1 }
            }
    };
}

static void
optimize_rope(int *rope_ptr, kdnode *node_vec, int index, KD_SIDE face) {
    if (*rope_ptr == -1) {
        return;
    }
    while (node_vec[*rope_ptr].type != KD_LEAF) {
        if (face / 2 == node_vec[*rope_ptr].split.axis) {
            break;
        }
        KD_AXIS axis = node_vec[*rope_ptr].split.axis;
        vec_t value = node_vec[*rope_ptr].split.value;
        if (value >= node_vec[index].max.s[axis]) {
            *rope_ptr = node_vec[*rope_ptr].split.children[0];
        } else if (value <= node_vec[index].min.s[axis]) {
            *rope_ptr = node_vec[*rope_ptr].split.children[1];
        } else {
            break;
        }
    }
}

static void
add_ropes(kdnode *node_vec, kd_index index, kd_index ropes[6]) {
    if (node_vec[index].type == KD_LEAF) {
        for (KD_SIDE side = 0; side < 6; side++) {
            node_vec[index].leaf.ropes[side] = ropes[side];
        }
        return;
    }
    int ropes0[6], ropes1[6];
    for (KD_SIDE face = 0; face < 6; face++) {
        optimize_rope(&ropes[face], node_vec, index, face);
        ropes0[face] = ropes[face];
        ropes1[face] = ropes[face];
    }
    KD_AXIS axis = node_vec[index].split.axis;
    ropes0[2 * axis + 1] = node_vec[index].split.children[1];
    add_ropes(node_vec, node_vec[index].split.children[0], ropes0);
    ropes1[2 * axis] = node_vec[index].split.children[0];
    add_ropes(node_vec, node_vec[index].split.children[1], ropes1);
}

static int *
concat_tris(int *tris, const triangle *new_tris) {
    size_t num_tris = vector_length(new_tris);
    for (size_t i = 0; i < num_tris; i++) {
        vector_append(tris, new_tris[i].index);
    }
    return tris; // May have moved if reallocated
}

static kd_index
new_build_tree(kd *tree,
        triangle *tris,
        Vector3 min,
        Vector3 max,
        KD_AXIS axis,
        int depth);

static kd_index
build_split(kd *tree,
        triangle *tris,
        Vector3 min,
        Vector3 max,
        KD_AXIS next_axis,
        int depth) {
    kd_index index = vector_length(tree->node_vec);
    size_t num_tris = vector_length(tris);
    Vector3 vmin = max, vmax = min, extents = vec_subtract(max, min);
    for (size_t i = 0; i < num_tris; i++) {
        vmin = vec_min(vmin, tris[i].V[0]);
        vmin = vec_min(vmin, tris[i].V[1]);
        vmin = vec_min(vmin, tris[i].V[2]);
        vmax = vec_max(vmax, tris[i].V[0]);
        vmax = vec_max(vmax, tris[i].V[1]);
        vmax = vec_max(vmax, tris[i].V[2]);
    }
    Vector3 smin = vec_divide(vec_subtract(max, vmin), extents);
    Vector3 smax = vec_divide(vec_subtract(vmax, min), extents);
    for (int axis = KD_X; axis <= KD_Z; axis++) {
        if (smin.s[axis] < 0.8) {
            vec_t v = vmin.s[axis];
            kd_index new_index = vector_length(tree->node_vec);
            vector_append(tree->node_vec, new_split(min, max, v, axis));
            Vector3 low_max = max;
            low_max.s[axis] = v;
            tree->node_vec[new_index].split.children[0] =
                    vector_length(tree->node_vec);
            vector_append(tree->node_vec, new_leaf(min, low_max, -1, 0));
            min.s[axis] = v;
            tree->node_vec[new_index].split.children[1] =
                    vector_length(tree->node_vec);
        }
        if (smax.s[axis] < 0.8) {
            vec_t v = vmax.s[axis];
            kd_index new_index = vector_length(tree->node_vec);
            vector_append(tree->node_vec, new_split(min, max, v, axis));
            Vector3 high_min = min;
            high_min.s[axis] = v;
            tree->node_vec[new_index].split.children[1] =
                    vector_length(tree->node_vec);
            vector_append(tree->node_vec, new_leaf(high_min, max, -1, 0));
            max.s[axis] = v;
            tree->node_vec[new_index].split.children[0] =
                    vector_length(tree->node_vec);
        }
    }
    new_build_tree(tree, tris, min, max, next_axis, depth);
    return index;
}

static kd_index
new_build_tree(kd *tree,
        triangle *tris,
        Vector3 min,
        Vector3 max,
        KD_AXIS axis,
        int depth) {
    size_t num_tris = vector_length(tris);
    kd_index index = vector_length(tree->node_vec);
    if (num_tris <= 1 || depth == 0) {
        if (depth != 0) {
            build_split(tree, tris, min, max, (axis + 1) % 3, 0);
            return index;
        }
        kd_index tri_index = (kd_index)vector_length(tree->tri_indices);
        vector_append(tree->node_vec, new_leaf(min, max, tri_index, num_tris));
        tree->tri_indices = concat_tris(tree->tri_indices, tris);
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
    //qsort_s(tris, num_tris, sizeof(*tris), tri_comp, &axis);
    //vec_t median = tris[num_tris / 2].center.s[axis];
    //vec_t *values = new_vector(3 * num_tris * sizeof(*values));
    vec_t value = 0;
    int count = 0;
    for (size_t i = 0; i < num_tris; i++) {
        for (size_t j = 0; j < 3; j++) {
            vec_t v = tris[i].V[j].s[axis];
            value += v;
            count++;
        }
    }
    //qsort(values, num_values, sizeof(*values), cmp_VEC_TYPE);
    //vec_t median = values[num_values / 2] + 0.0001f;
    vec_t median = value / (vec_t)count;
    if (median < min.s[axis] + 0.02 || median > max.s[axis] - 0.02) {
        build_split(tree, tris, min, max, (axis + 1) % 3, depth - 1);
        return index;
        //kd_index tri_index = (kd_index)vector_length(tree->tri_vec);
        //vector_append(tree->node_vec, new_leaf(min, max, tri_index,
        //num_tris));
        //tree->tri_vec = concat_tris(tree->tri_vec, tris);
        //return index;
    }
    triangle *low_tris = new_vector(num_tris * sizeof(*low_tris));
    triangle *high_tris = new_vector(num_tris * sizeof(*high_tris));
    Vector3 vmin = max, vmax = min;
    for (size_t i = 0; i < num_tris; i++) {
        int low = 0, high = 0;
        for (int j = 0; j < 3; j++) {
            if (tris[i].V[j].s[axis] <= median) {
                low = 1;
            }
            if (tris[i].V[j].s[axis] >= median) {
                high = 1;
            }
            vmin = vec_min(vmin, tris[i].V[j]);
            vmax = vec_max(vmax, tris[i].V[j]);
        }
        if (low) {
            vector_append(low_tris, tris[i]);
        }
        if (high) {
            vector_append(high_tris, tris[i]);
        }
    }

    Vector3 low_max = max, high_min = min;
    low_max.s[axis] = high_min.s[axis] = median;
    vector_append(tree->node_vec, new_split(min, max, median, axis));
    kd_index low_index = build_split(tree,
            low_tris,
            min,
            low_max,
            (axis + 1) % 3,
            depth - 1);
    kd_index high_index = build_split(tree,
            high_tris,
            high_min,
            max,
            (axis + 1) % 3,
            depth - 1);
    delete_vector(low_tris);
    delete_vector(high_tris);
    tree->node_vec[index].split.children[0] = low_index;
    tree->node_vec[index].split.children[1] = high_index;
    return index;
}

kd
build_kd(cl_int3 *tris, Vector3 *verts, Vector3 *norms, const char *path) {
    size_t num_tris = vector_length(tris);
    kd tree = {
            new_vector(0),
            new_vector(num_tris * sizeof(*tree.tri_indices)),
            verts,
            norms,
            tris
    };
    triangle *triangles = new_vector(num_tris * sizeof(*triangles));
    Vector3 min, max;
    min = max = verts[tris[0].s[0]];
    for (size_t i = 0; i < num_tris / 3; i++) {
        Vector3 A = verts[tris[3 * i + 0].s[0]],
                B = verts[tris[3 * i + 1].s[0]],
                C = verts[tris[3 * i + 2].s[0]];
        min = vec_min(min, vec_min(vec_min(A, B), C));
        max = vec_max(max, vec_max(vec_max(A, B), C));
        vector_append(triangles, ((triangle){
                i, {
                        A, B, C
                }, vec_scaled(vec_add(vec_add(A, B), C), 1.0f / 3.0f)
        }));
    }
    new_build_tree(&tree, triangles, min, max, KD_X, DEPTH);
    delete_vector(triangles);
    printf("%d %d %f\n",
            leafTriCount,
            leafCount,
            (double)leafTriCount / (double)leafCount);
    add_ropes(tree.node_vec, 0, (kd_index[6]){
            -1, -1, -1, -1, -1, -1
    });
    if (path) {
        size_t size = snprintf(NULL, 0, "%s.kd", path);
        char *kdpath = malloc(size + 1);
        if (kdpath == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        sprintf(kdpath, "%s.kd", path);
        FILE *file = fopen(kdpath, "wb");
        free(kdpath);
        size_t node_len = vector_length(tree.node_vec);
        fwrite(&node_len, sizeof(node_len), 1, file);
        fwrite(tree.node_vec, sizeof(*tree.node_vec), node_len, file);
        size_t vert_len = vector_length(tree.vert_vec);
        fwrite(&vert_len, sizeof(vert_len), 1, file);
        fwrite(tree.vert_vec, sizeof(*tree.vert_vec), vert_len, file);
        size_t norm_len = vector_length(tree.norm_vec);
        fwrite(&norm_len, sizeof(norm_len), 1, file);
        fwrite(tree.norm_vec, sizeof(*tree.norm_vec), norm_len, file);
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

    size_t norm_len;
    fread(&norm_len, sizeof(norm_len), 1, file);
    tree->norm_vec = init_vector(norm_len, sizeof(*tree->norm_vec));
    fread((Vector4 *)tree->norm_vec, sizeof(*tree->norm_vec), norm_len, file);

    size_t tri_len;
    fread(&tri_len, sizeof(tri_len), 1, file);
    tree->tri_vec = init_vector(tri_len, sizeof(*tree->tri_vec));
    fread(tree->tri_vec, sizeof(*tree->tri_vec), tri_len, file);

    return 0;
}

void
delete_kd(kd tree) {
    delete_vector(tree.node_vec);
    delete_vector(tree.tri_vec);
    delete_vector(tree.norm_vec);
    delete_vector(tree.vert_vec);
    delete_vector(tree.tri_indices);
}
