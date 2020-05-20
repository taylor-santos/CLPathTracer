#ifndef LIST_H
#define LIST_H

#include <stddef.h>

extern size_t LIST_INDEX;

/* Add 'item' to the end of the vector, resizing if necessary. Note: the
 * vector must be defined as a pointer to 'item's type, to allow for proper
 * type checking.
 */
#define vector_append(vec, item) ( \
        LIST_INDEX = list_grow((void**)&(vec), sizeof((item))), \
        (vec)[LIST_INDEX] = (item) \
    )
#define vector_length(vec) (list_size(vec) / sizeof(*vec))
#define vector_concat(v1, v2) vec_concat((void**)&v1, v2)
void
list_concat(void **list1_ptr, const void *list2);
void *
new_list(size_t capacity);
void *
init_list(size_t count, size_t size);
void *
copy_list(const void *);
void
delete_list(void *);
/* Add 'size' to the vector's length, reallocating if necessary. Returns the
 * vector's old length divided by 'size'. This can be used to index into the
 * vector at the newly added space. (see: vector_append())
 */
size_t
list_grow(void **list_ptr, size_t size);
size_t
list_size(const void *);

#endif//LIST_H
