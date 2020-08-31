#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "kd_tree.h"
#include "list.h"

#define DEPTH 15
#define NBINS 25
#define EPS   0.000000001

typedef struct triangle {
    int     index;
    vec_t   SA;
    Vector3 V[3], center;
} triangle;

static int leafCount    = 0;
static int leafTriCount = 0;

static kdnode
new_leaf(Vector3 min, Vector3 max, kd_index tris, kd_index tri_count) {
    leafCount++;
    leafTriCount += tri_count;
    return (kdnode){
        min,
        max,
        KD_LEAF,
        .leaf = {tris, tri_count, {-1, -1, -1, -1, -1, -1}}};
}

static kdnode
new_split(Vector3 min, Vector3 max, vec_t value, KD_AXIS axis) {
    return (kdnode){min, max, KD_SPLIT, .split = {value, axis, {-1, -1}}};
}

static void
optimize_rope(int *rope_ptr, kdnode *node_vec, int index, KD_SIDE face) {
    if (*rope_ptr == -1) { return; }
    while (node_vec[*rope_ptr].type != KD_LEAF) {
        if (face / 2 == node_vec[*rope_ptr].split.axis) { break; }
        KD_AXIS axis  = node_vec[*rope_ptr].split.axis;
        vec_t   value = node_vec[*rope_ptr].split.value;
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
    KD_AXIS axis         = node_vec[index].split.axis;
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

static void
SAH_tree(kd *tree, triangle *tris, Vector3 min, Vector3 max, int depth) {
    size_t num_tris = vector_length(tris);
    if (num_tris <= 1 || depth == 0) {
        kd_index tri_index = vector_length(tree->tri_indices);
        vector_append(tree->node_vec, new_leaf(min, max, tri_index, num_tris));
        tree->tri_indices = concat_tris(tree->tri_indices, tris);
        return;
    }
    Vector3 ext   = vec_subtract(max, min);
    int     found = 0;
    vec_t   best_h;
    KD_AXIS best_axis;
    vec_t   best_v;
    vec_t   box_a[] = {
        2 * ext.s[1] * ext.s[2],
        2 * ext.s[0] * ext.s[2],
        2 * ext.s[0] * ext.s[1]};
    for (KD_AXIS axis = 0; axis < 3; axis++) {
        vec_t e = ext.s[axis];
        if (e < EPS) { continue; }
        for (int i = 0; i < NBINS; i++) {
            vec_t d = (vec_t)(i + 1) / (vec_t)(NBINS + 1);
            vec_t v = min.s[axis] + d * e;
            vec_t SL =
                      2 *
                      (ext.s[(axis + 1) % 3] * ext.s[(axis + 2) % 3] +
                       e * d * (ext.s[(axis + 1) % 3] + ext.s[(axis + 2) % 3])),
                  SR =
                      2 * (ext.s[(axis + 1) % 3] * ext.s[(axis + 2) % 3] +
                           e * (1 - d) *
                               (ext.s[(axis + 1) % 3] + ext.s[(axis + 2) % 3]));
            int NL = 0, NR = 0;
            for (size_t t = 0; t < num_tris; t++) {
                triangle tri = tris[t];
                int      isL = 0, isR = 0;
                for (int j = 0; j < 3 && (!isL || !isR); j++) {
                    if (tri.V[j].s[axis] <= v) { isL = 1; }
                    if (tri.V[j].s[axis] >= v) { isR = 1; }
                }
                if (isL) {
                    NL++;
                    SL += tri.SA;
                }
                if (isR) {
                    NR++;
                    SR += tri.SA;
                }
                if (found && NL * SL + NR * SR >= best_h) { break; }
            }
            if (!found || NL * SL + NR * SR < best_h) {
                found     = 1;
                best_h    = NL * SL + NR * SR;
                best_axis = axis;
                best_v    = v;
            }
        }
    }
    if (!found || best_v <= min.s[best_axis] || max.s[best_axis] <= best_v) {
        kd_index tri_index = vector_length(tree->tri_indices);
        vector_append(tree->node_vec, new_leaf(min, max, tri_index, num_tris));
        tree->tri_indices = concat_tris(tree->tri_indices, tris);
        return;
    }
    triangle *L_tris = new_list(sizeof(*L_tris) * num_tris),
             *R_tris = new_list(sizeof(*R_tris) * num_tris);
    for (size_t i = 0; i < num_tris; i++) {
        triangle tri = tris[i];
        int      isL = 0, isR = 0;
        for (int j = 0; j < 3 && (!isL || !isR); j++) {
            if (tri.V[j].s[best_axis] <= best_v + EPS) { isL = 1; }
            if (tri.V[j].s[best_axis] >= best_v - EPS) { isR = 1; }
        }
        if (isL) { vector_append(L_tris, tri); }
        if (isR) { vector_append(R_tris, tri); }
    }
    Vector3 L_max = max, R_min = min;
    L_max.s[best_axis] = R_min.s[best_axis] = best_v;

    kd_index index = vector_length(tree->node_vec);
    vector_append(tree->node_vec, new_split(min, max, best_v, best_axis));

    kd_index L_index = vector_length(tree->node_vec);
    SAH_tree(tree, L_tris, min, L_max, depth - 1);
    delete_list(L_tris);

    kd_index R_index = vector_length(tree->node_vec);
    SAH_tree(tree, R_tris, R_min, max, depth - 1);
    delete_list(R_tris);

    tree->node_vec[index].split.children[0] = L_index;
    tree->node_vec[index].split.children[1] = R_index;
}

kd
build_kd(cl_int3 *tris, Vector3 *verts, Vector3 *norms, const char *path) {
    size_t num_tris = vector_length(tris);
    kd     tree     = {
        new_list(0),
        new_list(num_tris * sizeof(*tree.tri_indices)),
        verts,
        norms,
        tris};
    triangle *triangles = new_list(num_tris * sizeof(*triangles));
    Vector3   min, max;
    min = max = verts[tris[0].s[0]];
    for (size_t i = 0; i < num_tris / 3; i++) {
        Vector3 A  = verts[tris[3 * i + 0].s[0]],
                B  = verts[tris[3 * i + 1].s[0]],
                C  = verts[tris[3 * i + 2].s[0]];
        Vector3 S1 = vec_subtract(B, A), S2 = vec_subtract(C, A);
        Vector3 N = vec_cross(S1, S2);
        min       = vec_min(min, vec_min(vec_min(A, B), C));
        max       = vec_max(max, vec_max(vec_max(A, B), C));
        vector_append(
            triangles,
            ((triangle){
                i,
                vec_length(N) / 2,
                {A, B, C},
                vec_scaled(vec_add(vec_add(A, B), C), 1.0f / 3.0f)}));
    }
    SAH_tree(&tree, triangles, min, max, DEPTH);
    // new_build_tree(&tree, triangles, min, max, KD_X, DEPTH);
    delete_list(triangles);
    printf(
        "%d %d %f\n",
        leafTriCount,
        leafCount,
        (double)leafTriCount / (double)leafCount);
    add_ropes(tree.node_vec, 0, (kd_index[6]){-1, -1, -1, -1, -1, -1});
    if (path) {
        size_t size   = snprintf(NULL, 0, "%s.kd", path);
        char * kdpath = malloc(size + 1);
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

        size_t tri_index_len = vector_length(tree.tri_indices);
        fwrite(&tri_index_len, sizeof(tri_index_len), 1, file);
        fwrite(
            tree.tri_indices,
            sizeof(*tree.tri_indices),
            tri_index_len,
            file);

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
    tree->node_vec = init_list(node_len, sizeof(*tree->node_vec));
    fread(tree->node_vec, sizeof(*tree->node_vec), node_len, file);

    size_t vert_len;
    fread(&vert_len, sizeof(vert_len), 1, file);
    tree->vert_vec = init_list(vert_len, sizeof(*tree->vert_vec));
    fread((Vector4 *)tree->vert_vec, sizeof(*tree->vert_vec), vert_len, file);

    size_t norm_len;
    fread(&norm_len, sizeof(norm_len), 1, file);
    tree->norm_vec = init_list(norm_len, sizeof(*tree->norm_vec));
    fread((Vector4 *)tree->norm_vec, sizeof(*tree->norm_vec), norm_len, file);

    size_t tri_index_len;
    fread(&tri_index_len, sizeof(tri_index_len), 1, file);
    tree->tri_indices = init_list(tri_index_len, sizeof(*tree->tri_indices));
    fread(
        (Vector4 *)tree->tri_indices,
        sizeof(*tree->tri_indices),
        tri_index_len,
        file);

    size_t tri_len;
    fread(&tri_len, sizeof(tri_len), 1, file);
    tree->tri_vec = init_list(tri_len, sizeof(*tree->tri_vec));
    fread(tree->tri_vec, sizeof(*tree->tri_vec), tri_len, file);

    return 0;
}

void
delete_kd(kd tree) {
    delete_list(tree.node_vec);
    delete_list(tree.tri_vec);
    delete_list(tree.norm_vec);
    delete_list(tree.vert_vec);
    delete_list(tree.tri_indices);
}
