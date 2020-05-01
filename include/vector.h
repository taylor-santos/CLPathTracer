#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef void *vector;

vector
new_vector(void);

void
delete_vector(vector);

/* Add 'size' to the vector's length, reallocating if necessary. Returns the
 * vector's old length divided by 'size'. This can be used to index into the
 * vector at the newly added space. (see: vector_append())
 */
size_t
vector_grow(vector *vec_ptr, size_t size);

extern size_t VEC_INDEX;
/* Add 'item' to the end of the vector, resizing if necessary. Note: the
 * vector must be defined as a pointer to 'item's type, to allow for proper
 * type checking.
 */
#define vector_append(vec, item) ( \
        VEC_INDEX = vector_grow((void**)&(vec), sizeof(item)), \
        (vec)[VEC_INDEX] = (item) \
    )

size_t
vector_size(vector);

#define vector_length(vec) \
    (vector_size(vec) / sizeof(*vec))

#endif//VECTOR_H
