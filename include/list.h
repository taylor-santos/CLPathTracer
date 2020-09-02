#ifndef LIST_H
#define LIST_H

#include <stddef.h>

extern size_t LIST_INDEX;

/* Add 'item' to the end of the vector, resizing if necessary. Note: the
 * vector must be defined as a pointer to 'item's type, to allow for proper
 * type checking.
 */
#define list_append(list, item)                                        \
    (LIST_INDEX         = list_grow((void **)&(list), sizeof((item))), \
     (list)[LIST_INDEX] = (item))
#define list_length(list)   (list_size(list) / sizeof(*list))
#define list_concat(v1, v2) internal_list_concat((void **)&v1, v2)
void
internal_list_concat(void **pList1, const void *list2);
void *
new_list(size_t capacity);
void *
init_list(size_t count, size_t size);
void *
copy_list(const void *list);
void
delete_list(void *list);
/* Add 'size' to the vector's length, reallocating if necessary. Returns the
 * vector's old length divided by 'size'. This can be used to index into the
 * vector at the newly added space. (see: vector_append())
 */
size_t
list_grow(void **pList, size_t size);
size_t
list_size(const void *list);

#endif // LIST_H
