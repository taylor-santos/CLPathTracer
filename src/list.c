#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct data data;

size_t LIST_INDEX;

struct data {
    size_t capacity;
    size_t length;
    char data[];
};

static data *
get_list(void *list) {
    data *l = list;
    return &l[-1];
}

static const data *
get_const_list(const void *list) {
    const data *l = list;
    return &l[-1];
}

void *
new_list(size_t capacity) {
    data *l;
    if (NULL == (l = malloc(sizeof(*l) + capacity))) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *l = (data){
            capacity, 0
    };
    return l->data;
}

void *
init_list(size_t count, size_t size) {
    data *l;
    if (NULL == (l = malloc(sizeof(*l) + count * size))) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *l = (data){
            count * size, count * size
    };
    return l->data;
}

void *
copy_list(const void *list) {
    const data *l = get_const_list(list);
    void *copy = init_list(1, l->length);
    memcpy(copy, l->data, l->length);
    data *copy_data = get_list(copy);
    copy_data->length = l->length;
    return copy;
}

void
delete_list(void *list) {
    if (list == NULL) {
        return;
    }
    data *l = get_list(list);
    free(l);
}

static void
list_realloc(data **lptr, size_t size) {
    void *new_l = realloc(*lptr, sizeof(data) + size);
    if (new_l == NULL) {
        free(*lptr);
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    *lptr = new_l;
    (*lptr)->capacity = size;
}

void
list_concat(void **list1_ptr, const void *list2) {
    data *l1 = get_list(*list1_ptr);
    const data *l2 = get_const_list(list2);
    size_t s1 = l1->length, s2 = l2->length;
    list_realloc(&l1, s1 + s2);
    memcpy(l1->data + s1, l2->data, s2);
    l1->length = s1 + s2;
    l1->capacity = s1 + s2;
    *list1_ptr = l1->data;
}

size_t
list_grow(void **list_ptr, size_t size) {
    data *l = get_list(*list_ptr);
    if (l->length + size > l->capacity) {
        list_realloc(&l, l->capacity * 2 + l->length + size);
    }
    l->length += size;
    *list_ptr = l->data;
    return l->length / size - 1;
}

size_t
list_size(const void *list) {
    const data *l = get_const_list(list);
    return l->length;
}
