#include "voxel.h"
#include "list.h"

#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

static size_t max_index = 0;

static void
voxel_add(
    Voxel **     p_voxels,
    unsigned int x,
    unsigned int y,
    unsigned int z,
    cl_float4    color) {
    assert(
        x < (1u << VOXEL_DEPTH) && y < (1u << VOXEL_DEPTH) &&
        z < (1u << VOXEL_DEPTH));
    Voxel *curr = *p_voxels;
    for (unsigned int depth = 0; depth < VOXEL_DEPTH; depth++) {
        unsigned int bounds = 1u << (VOXEL_DEPTH - depth);
        unsigned int index  = 0;
        if (x >= bounds / 2) {
            index += 4;
            x -= bounds / 2;
        }
        if (y >= bounds / 2) {
            index += 2;
            y -= bounds / 2;
        }
        if (z >= bounds / 2) {
            index++;
            z -= bounds / 2;
        }
        if (curr->internal.children[index] == 0) {
            size_t new_index = list_length(*p_voxels);
            if (new_index > max_index) max_index = new_index;
            curr->internal.children[index] = new_index;
            list_append(*p_voxels, ((Voxel){INTERNAL, {.internal = {{0}}}}));
            curr = *p_voxels + new_index;
        } else {
            curr = *p_voxels + curr->internal.children[index];
        }
    }
    curr->type       = LEAF;
    curr->leaf.color = color;
}

/* https://stackoverflow.com/a/9493060 */
static float
hue2rgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

static Vector3
hsl2rgb(Vector3 hsl) {
    float r, g, b;
    float h = vec_x(hsl), s = vec_y(hsl), l = vec_z(hsl);
    if (s == 0) {
        r = g = b = l;
    } else {
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        r       = hue2rgb(p, q, h + 1.0f / 3.0f);
        g       = hue2rgb(p, q, h);
        b       = hue2rgb(p, q, h - 1.0f / 3.0f);
    }
    return Vector3(r, g, b);
}

Vector3
rgb2hsl(Vector3 rgb) {
    float h, s, l;
    float r = vec_x(rgb), g = vec_y(rgb), b = vec_z(rgb);
    float max = (r > g && r > b) ? r : (g > b) ? g : b;
    float min = (r < g && r < b) ? r : (g < b) ? g : b;

    l = (max + min) / 2.0f;

    if (max == min) {
        h = s = 0.0f;
    } else {
        float d = max - min;
        s       = (l > 0.5f) ? d / (2.0f - max - min) : d / (max + min);

        if (r > g && r > b) h = (g - b) / d + (g < b ? 6.0f : 0.0f);

        else if (g > b)
            h = (b - r) / d + 2.0f;

        else
            h = (r - g) / d + 4.0f;

        h /= 6.0f;
    }
    return Vector3(h, s, l);
}

Voxel *
voxel_generate(void) {
    srand(1234);
    Voxel *voxels = new_list(sizeof(Voxel));
    Voxel  root   = (Voxel){INTERNAL, {.internal = {{0}}}};
    list_append(voxels, root);
    unsigned int bounds = 1u << VOXEL_DEPTH;
    voxel_add(&voxels, 0, 0, 0, (cl_float4){{1, 0, 0, 1}});
    voxel_add(
        &voxels,
        bounds - 1,
        bounds - 1,
        bounds - 1,
        (cl_float4){{0, 1, 0, 1}});
    for (int x = 0; x < bounds; x++) {
        for (int y = 0; y < bounds; y++) {
            Vector4 color;

            color = hsl2rgb(Vector3(
                rand() / (float)RAND_MAX,
                0.25f + (0.95f - 0.25f) * (rand() / (float)RAND_MAX),
                0.65f + (0.85f - 0.65f) * (rand() / (float)RAND_MAX)));
            voxel_add(&voxels, x, y, 0, color);
            // voxel_add( &voxels, x, 0, y, color);
            color = hsl2rgb(Vector3(rand() / (float)RAND_MAX, 0.97f, 0.79f));
            voxel_add(&voxels, 0, x, y, color);
            color = hsl2rgb(Vector3(rand() / (float)RAND_MAX, 0.97f, 0.79f));
            voxel_add(
                &voxels,
                bounds - 1 - x,
                bounds - 1 - y,
                bounds - 1 - 0,
                color);
            // voxel_add(
            //    &voxels,
            //    bounds - 1 - x,
            //    bounds - 1 - 0,
            //    bounds - 1 - y,
            //    (cl_float4){
            //        {rand() / (float)RAND_MAX,
            //         rand() / (float)RAND_MAX,
            //         rand() / (float)RAND_MAX,
            //         1}});
            color = hsl2rgb(Vector3(rand() / (float)RAND_MAX, 0.97f, 0.79f));
            voxel_add(
                &voxels,
                bounds - 1 - 0,
                bounds - 1 - x,
                bounds - 1 - y,
                color);
        }
    }
    for (int i = 0; i < 10000; i++) {
        voxel_add(
            &voxels,
            4 + rand() % (bounds - 8),
            4 + rand() % (bounds - 8),
            4 + rand() % (bounds - 8),
            (cl_float4){
                {rand() / (float)RAND_MAX / 2,
                 rand() / (float)RAND_MAX / 2,
                 rand() / (float)RAND_MAX / 2,
                 1}});
    }
    // for (int i = 0; i < 1u << VOXEL_DEPTH; i++) {
    //    voxel_add(&voxels, 0, 5, i, (cl_float4){{1, 1, 1, 1}});
    //}
    return voxels;
}
