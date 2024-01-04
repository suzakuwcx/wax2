#ifndef _LIBWAX_H_
#define _LIBWAX_H_ 1

#include <stdbool.h>
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
 * @brief get an element from vector
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
 * @param size The memory size of the object
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
 * @brief Run command with format string
 * 
 * @param format The format string, same as 'printf'
 * @param ... The argument list
 * @return int The return value of 'system()' function
 *
 * @code
 * systemf("ls %s", "/tmp");
 * @endcode
 * 
 */
int systemf(const char *restrict format, ...);


/**
 * @brief Create directory like 'mkdir -p <path>'
 * 
 * @param path The target path to be created
 * @return 0 If everything ok
 * @return 1 If error occur
 * 
 * @code
 * // create directory /a/b/c/d/e/f
 * ret = mkdir_p("a/b/c/d/e/f");
 * if (ret < 0)
 *     fprintf(stderr, "create directory failed\n");
 * @endcode 
 */
int mkdir_p(const char *path);


/**
 * @brief Move directory like 'mv <old> <new>'
 * 
 * @param old Old path
 * @param new New path
 * @return 0 If everything ok
 * @return 1 If error occur
 *
 * @code
 * ret = mv("dir", "/tmp");
 * if (ret < 0)
 *     fprintf(stderr, "move directory failed\n");
 * @endcode
 */
int mv(const char *old, const char *new);


/**
 * @brief Copy file like 'cp -r <src> <dst>'
 * 
 * @param src The source file location
 * @param dst The destination file location
 * @return 0 If everything ok
 * @return 1 If error occur
 *
 * @code
 * ret = cp_r("dir", "/tmp");
 * if (ret < 0)
 *     fprintf(stderr, "move directory failed\n");
 * @endcode
 */
int cp_r(const char *src, const char *dst);


/**
 * @brief Remove file like 'rm -r <path>'
 * 
 * @param path The file that will be remove recursion
 * @return 0 If everything ok
 * @return 1 If error occur
 * @code
 * ret = cp_r("dir", "/tmp");
 * if (ret < 0)
 *     fprintf(stderr, "move directory failed\n");
 * @endcode
 */
int rm_r(const char *path);


/**
 * @brief Test if a C string a valid natural number
 * 
 * @param str A c string to be test
 * @return true This string is natural number
 * @return false This string is not an natural number
 * @code
 * is_string_number("12345"); // return true
 * is_string_number("-1345"); // return false
 * is_string_number("0x1A"); // return false
 * @endcode
 */
bool is_string_number(const char *str);


struct dfa;


struct dfa *new_dfa(char *regex);
int dfa_check(struct dfa *d, char *buf, size_t len);
void dfa_reset(struct dfa *d);
void dfa_delete(struct dfa *d);


void clear();
void clear_line();


#endif /* _LIBWAX_H_ */
