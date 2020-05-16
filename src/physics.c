#include <stdlib.h>

#include "physics.h"
#include "vector.h"

#define REALLOC_SIZE 16

typedef struct PhysObject {
    Vector3 *position, *velocity;
} PhysObject;

typedef struct PhysPtr {
    void **pos_ptr;
    ptrdiff_t pos_offset;
    void **vel_ptr;
    ptrdiff_t vel_offset;
} PhysPtr;

static PhysObject *objects = NULL;
static size_t obj_count = 0;

static PhysPtr *ptrs = NULL;
static size_t ptr_count = 0;

void
AddPhysObject(Vector3 *position, Vector3 *velocity) {
    if (objects == NULL) {
        objects = new_vector(sizeof(*objects));
    }
    vector_append(objects, ((PhysObject){
            position, velocity
    }));
    obj_count++;
}

void
AddPhysPtr(void *pos_base, void *pos_ptr, void *vel_base, void *vel_ptr) {
    if (ptrs == NULL) {
        ptrs = new_vector(sizeof(*ptrs));
    }
    ptrdiff_t pos_diff = (char *)pos_ptr - *(char **)pos_base;
    ptrdiff_t vel_diff = (char *)vel_ptr - *(char **)vel_base;
    vector_append(ptrs, ((PhysPtr){
            pos_base, pos_diff, vel_base, vel_diff
    }));
    ptr_count++;
}

void
PhysStep(double stepSize) {
    for (size_t i = 0; i < obj_count; i++) {
        *objects[i].position = vec_add(*objects[i].position,
                vec_scaled(*objects[i].velocity, stepSize));
    }
    for (size_t i = 0; i < ptr_count; i++) {
        char *ptr = *ptrs[i].pos_ptr;
        ptr += ptrs[i].pos_offset;
        Vector3 *pos = (Vector3 *)ptr;
        ptr = *ptrs[i].vel_ptr;
        ptr += ptrs[i].vel_offset;
        Vector3 *vel = (Vector3 *)ptr;
        *pos = vec_add(*pos, vec_scaled(*vel, stepSize));
    }
}

void
PhysTerminate(void) {
    delete_vector(objects);
    delete_vector(ptrs);
}
