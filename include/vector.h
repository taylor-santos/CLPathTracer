#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

extern size_t VEC_INDEX;

/* Add 'item' to the end of the vector, resizing if necessary. Note: the
 * vector must be defined as a pointer to 'item's type, to allow for proper
 * type checking.
 */
#define vector_append(vec, item) ( \
        VEC_INDEX = vector_grow((void**)&(vec), sizeof((item))), \
        (vec)[VEC_INDEX] = (item) \
    )
#define vector_length(vec) (vector_size(vec) / sizeof(*vec))
#define vector_concat(v1, v2) vec_concat((void**)&v1, v2)
void
vec_concat(void *v1_ptr, const void *v2);
void *
new_vector(size_t capacity);
void *
init_vector(size_t count, size_t size);
void *
copy_vector(const void *);
void
delete_vector(void *);
/* Add 'size' to the vector's length, reallocating if necessary. Returns the
 * vector's old length divided by 'size'. This can be used to index into the
 * vector at the newly added space. (see: vector_append())
 */
size_t
vector_grow(void **vec_ptr, size_t size);
size_t
vector_size(const void *);

#endif//VECTOR_H
