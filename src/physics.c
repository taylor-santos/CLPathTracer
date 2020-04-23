#include <stdlib.h>

#include "physics.h"

#define REALLOC_SIZE 16

static struct {
    Vector3 *position, *velocity;
} *objects = NULL;
static size_t obj_count = 0, obj_cap = 0;

void
AddPhysObject(Vector3 *position, Vector3 *velocity) {
    if (obj_count == obj_cap) {
        obj_cap += REALLOC_SIZE;
        objects = realloc(objects, obj_cap * sizeof(*objects));
        if (objects == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
    }
    objects[obj_count].position = position;
    objects[obj_count].velocity = velocity;
    obj_count++;
}

void
PhysStep(double stepSize) {
    for (size_t i = 0; i < obj_count; i++) {
        *objects[i].position = vec_add(*objects[i].position,
            vec_scaled(*objects[i].velocity, stepSize));
    }
}

void
PhysTerminate(void) {
    free(objects);
}
