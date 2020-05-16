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

static data *
get_vector(void *vec) {
    data *data = vec;
    return &data[-1];
}

static const data *
get_const_vector(const void *vec) {
    const data *data = vec;
    return &data[-1];
}

void *
new_vector(size_t capacity) {
    data *vector;
    if (NULL == (vector = malloc(sizeof(*vector) + capacity))) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *vector = (data){
            capacity, 0
    };
    return vector->data;
}

void *
init_vector(size_t count, size_t size) {
    data *vector;
    if (NULL == (vector = malloc(sizeof(*vector) + count * size))) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *vector = (data){
            count * size, count * size
    };
    return vector->data;
}

void *
copy_vector(const void *v) {
    const data *vector = get_const_vector(v);
    void *copy = init_vector(1, vector->length);
    memcpy(copy, vector->data, vector->length);
    data *copy_data = get_vector(copy);
    copy_data->length = vector->length;
    return copy;
}

void
delete_vector(void *vector) {
    if (vector == NULL) {
        return;
    }
    data *vec = get_vector(vector);
    free(vec);
}

static void
vector_realloc(data **vec_ptr, size_t size) {
    void *new_vec = realloc(*vec_ptr, sizeof(data) + size);
    if (new_vec != *vec_ptr) {
        printf("Reallocating %zu\n", size);
    }
    if (new_vec == NULL) {
        free(*vec_ptr);
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    *vec_ptr = new_vec;
    (*vec_ptr)->capacity = size;
}

void
vec_concat(void **v1_ptr, const void *v2) {
    data *vec1 = get_vector(*v1_ptr);
    const data *vec2 = get_const_vector(v2);
    size_t s1 = vec1->length, s2 = vec2->length;
    vector_realloc(&vec1, s1 + s2);
    memcpy(vec1->data + s1, vec2->data, s2);
    vec1->length = s1 + s2;
    vec1->capacity = s1 + s2;
    *v1_ptr = vec1->data;
}

size_t
vector_grow(void **vec_ptr, size_t size) {
    data *vector = get_vector(*vec_ptr);
    if (vector->length + size > vector->capacity) {
        vector_realloc(&vector, vector->capacity * 2 + vector->length + size);
    }
    vector->length += size;
    *vec_ptr = vector->data;
    return vector->length / size - 1;
}

size_t
vector_size(const void *vec) {
    const data *vector = get_const_vector(vec);
    return vector->length;
}
