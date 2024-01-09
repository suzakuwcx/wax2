#include <stddef.h>

struct vector;

/**
 * @brief Create a new vector object, it can dynamicly grow
 * 
 * @param obj_delete the delete function that use to free the element store
 *                   in the array, if not special free function, leave NULL
 * @return struct vector* If success
 * @return NULL, If error occur
 * @code
 * struct vector *vec = new_vector(NULL);
 * 
 * void foo_delete(struct foo *f) {
 *     free(f->prt);
 *     free(f);
 * }
 * 
 * struct vector *vec = new_vector(foo_delete);
 * @endcode
 * 
 */
struct vector *new_vector(void (*obj_delete)(void *));


/**
 * @brief get an element from vector, the ptr that returned must not be freed
 * 
 * @param vec Vector object, created by 'new_vector()'
 * @param i the index
 * @return void* The object store in the index 'i', no need to free
 * @return NULL If the index out of bound or no object store in index i
 * @code
 * struct vector *vec = new_vector(NULL);
 * vector_push("Hello");
 * char *str = vector_get(vec, 0);
 * @endcode
 */
void *vector_get(struct vector *vec, int i);


/**
 * @brief Set the value of the element into vector at index 'i'
 *        This will copy the value from buf into the vector
 * 
 * @param vec Vector object, created by 'new_vector()'
 * @param i The index
 * @param buf The object that to be set
 * @param size The memory size of the object
 * @return 0 If success
 * @return 1 If index out of bound
 *
 * @code
 * struct vector *vec = new_vector(NULL);
 * char *str = "Helloworld";
 * vector_push("Hello");
 * vector_set(vec, 0, str, sizeof(str));
 * char *str = vector_get(vec, 0);
 * @endcode
 * 
 */
int vector_set(struct vector *vec, int i, const void *buf, size_t size);


/**
 * @brief Push an object to the end of the vector
 * 
 * @param vec Vector object, created by 'new_vector()'
 * @param buf The object that to be set
 * @param size The memory size of the object, if size is 0, treat object is 
 *              a c string and judge the size by 'strlen()'
 * @return 0 If success
 *
 * @code
 * struct vector *vec = new_vector(NULL);
 * vector_push("Hello");
 * char *str = vector_get(vec, 0);
 * @endcode
 * 
 */
int vector_push(struct vector *vec, const void *buf, size_t size);


/**
 * @brief Delete the vector object, free entire memory
 * 
 * @param vec Vector object, created by 'new_vector()'
 *
 * @code
 * struct vector *vec = new_vector(NULL);
 * vector_push("Hello");
 * vector_delete(vec);
 * @endcode
 */
void vector_delete(struct vector *vec);


/**
 * @brief Get the element number in vector
 * 
 * @param vec Vector object, created by 'new_vector()'
 * @return int The element number in vector
 *
 * @code
 * struct vector *vec = new_vector(NULL);
 * vector_push("Hello");
 * for (int i = 0; i < vector_len(vec); ++i) {
 *     printf("str: %s\n", vector_get(vec, i));
 * }
 * @endcode
 */
int vector_len(struct vector *vec);


/**
 * @brief convert vector to a constant object array, return value need to be
 *         freed by 'vector_obj_array_delete()'
 * 
 * @param vec Vector object, created by 'new_vector()'
 * @param size The memory size of the object, if size is 0, treat object is 
 *              a c string and judge the size by 'strlen()'
 * @return ptr It contains all the values store in vector, caller shouldn't 
 *          try to free or modify any of the value in it.
 *
 * @code
 * const char * const *arrays = vector_to_obj_array(vec, 0);
 * for (int i = 0; i < vector_len(vec); ++i)
 *     printf("string: %s", arrays[0]);
 * vector_obj_array_delete(arrays, 0);
 * @endcode
 */
const void * const * vector_to_obj_array(struct vector *vec, size_t size);


/**
 * @brief free memory allocd by 'vector_to_obj_array()'
 * 
 * @param arrays the arrays returned by 'vector_to_obj_array()'
 * @param size The memory size of the object, if size is 0, treat object is 
 *              a c string and judge the size by 'strlen()
 */
void vector_obj_array_delete(void **arrays, size_t size);


/**
 * @brief find the index of the object in vector
 * 
 * @param vec Vector object, created by 'new_vector()'
 * @param obj The object that to be found
 * @param size The memory size of the object, if size is 0, treat object is 
 *              a c string and judge the size by 'strlen()'
 * @return int Find the object at the index
 * @return -1 Object not found
 */
int vector_find(struct vector *vec, const void *obj, size_t size);
