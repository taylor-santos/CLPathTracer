#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct data data;

size_t VEC_INDEX;

struct data {
    size_t capacity;
    size_t length;
    char data[];
};

data *
get_vector(void *vec) {
    data *data = vec;
    return &data[-1];
}

void *
new_vector(void) {
    data *vector;
    if (NULL == (vector = malloc(sizeof(*vector)))) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *vector = (data){
        0,
        0
    };
    return vector->data;
}

void
delete_vector(void *vector) {
    if (vector == NULL) {
        return;
    }
    free(get_vector(vector));
}

static void
vector_realloc(data **vec_ptr, size_t size) {
    *vec_ptr = realloc(*vec_ptr, sizeof(data) + size);
    if (*vec_ptr == NULL) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    (*vec_ptr)->capacity = size;
}

size_t
vector_grow(void **vec_ptr, size_t size) {
    data *vector = get_vector(*vec_ptr);
    if (vector->length + size > vector->capacity) {
        vector_realloc(&vector, vector->length + size * 4);
    }
    vector->length += size;
    *vec_ptr = vector->data;
    return vector->length / size - 1;
}

size_t
vector_size(void *vec) {
    data *vector = get_vector(vec);
    return vector->length;
}
