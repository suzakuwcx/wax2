#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include <stddef.h>
#include <stdlib.h>

#include <wax/vector.h>

struct vector {
    void **ptr;                 /* object array */
    void (*obj_delete)(void *); /* obj delete function */
    int len;                    /* size of the object array that contain value */
    int maxlen;                 /* the maximun number that can save the element */
};


/*
 * free element at location i
 */
static inline void vector_delete_i(struct vector *vec, int i)
{
    if (vec->obj_delete == NULL)
        free((vec->ptr)[i]);
    else
        vec->obj_delete((vec->ptr)[i]);
}


/*
 * if vector is full, expand the space of the vector
 */
static int vector_resize(struct vector *vec)
{
    void **ptr = vec->ptr;
    vec->maxlen = vec->maxlen * 2;
    vec->ptr = reallocarray(ptr, vec->maxlen, sizeof(void *));

    return 0;
}


struct vector *new_vector(void (*obj_delete)(void *))
{
    struct vector *vec;
    vec = calloc(1, sizeof(struct vector));
    vec->len = 0;
    vec->maxlen = 10;

    vec->ptr = calloc(vec->maxlen, sizeof(void *));
    vec->obj_delete = obj_delete;
    return vec;
}


/*
 * get function return the pointer of the value, caller can modify the content
 * itself, but cannot free the pointer, or expand the space, example
 *
 * struct foo *f = vector_get(vec, 0);
 * foo_add_value(f); // modify the value object foo
 * // free(f) : error, must not free the pointer
 */ 
void *vector_get(struct vector *vec, int i)
{
    if (i >= vec->len)
        return NULL;

    return (vec->ptr)[i];
}


/*
 * set function will allocate memory by vector itself, and make a memory copy 
 * to store the value; So the origin pointer can still managed by user
 * 
 * after setting a new value, the memory size is fixed, cannot be expand or
 * shrink
 */
int vector_set(struct vector *vec, int i, const void *buf, size_t size)
{
    if (i >= vec->len)
        return -1;

    if ((vec->ptr)[i] != NULL)
        vector_delete_i(vec, i);

    (vec->ptr)[i] = calloc(1, size);
    memcpy((vec->ptr)[i], buf, size);

    return 0;
}


/*
 * If vector is full, do a resize
 */
int vector_push(struct vector *vec, const void *buf, size_t size)
{
    if (vec->len == vec->maxlen)
        vector_resize(vec);

    if (size == 0) {
        vec->ptr[vec->len] = strdup(buf);
    }
    else {
        vec->ptr[vec->len] = calloc(1, size);
        memcpy((vec->ptr)[vec->len], buf, size);
    }

    ++(vec->len);

    return 0;
}


void vector_delete(struct vector *vec)
{
    for (int i = 0; i < vec->len; ++i)
        vector_delete_i(vec, i);

    free(vec->ptr);
    free(vec);
}


inline int vector_len(struct vector *vec)
{
    return vec->len;
}


int vector_find(struct vector *vec, const void *obj, size_t size)
{
    for (int i = 0; i < vec->len; ++i) {
        if (size == 0) {
            if (strcmp(vec->ptr[i], obj) == 0)
                return i;
        }
        else if (memcmp(vec->ptr[i], obj, size) == 0) {
            return i;
        }
    }
    return -1;
}


const void * const * vector_to_obj_array(struct vector *vec, size_t size)
{
    void **arrays;

    arrays = calloc(vector_len(vec), sizeof(void *));

    for (int i = 0; i < vector_len(vec); ++i) {
        if (size == 0) {
            arrays[i] = calloc(1, strlen(vector_get(vec, i)) + 1);
            strcpy(arrays[i], vector_get(vec, i));
        }
        else {
            arrays[i] = calloc(1, size);
            memcpy(arrays[i], vector_get(vec, i), size);
        }
    }

    return (const void * const * )arrays;
}


void vector_obj_array_delete(void **arrays, size_t size)
{
    for (int i = 0; i < size; ++i)
        free(arrays[i]);
    
    free(arrays);
}

