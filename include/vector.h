#ifndef VECTOR_H
#define VECTOR_H

typedef void *vector;

vector
new_vector(void);

void
delete_vector(vector);

void *
vector_add_size(vector *vec_ptr, size_t size);

#define vector_append(vec, type, item) \
    (*(type*)vector_add_size((void**)&(vec), sizeof(type)) = (item))

size_t
vector_size(vector);

#define vector_length(vec, type) \
    vector_size(vec) / sizeof(type)

#endif//VECTOR_H
