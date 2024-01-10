#ifndef _HASHMAP_H_
#define _HASHMAP_H_ 1

#include <wax/list.h>
#include <stddef.h>


/*
 * Separate Chaining hashmap
 */
struct hashmap;


/**
 * @brief create a hashmap object
 * 
 * @return struct hashmap* The hashmap object instance
 */
struct hashmap *new_hashmap();


/**
 * @brief Create a hashmap object, with slot number size
 * 
 * The larger the size, the lower the probability of conflict. But as a 
 * trade-off, more wasted memory is required
 *
 * @param size The slot number of the hashmap
 * @return struct hashmap* The hashmap object instance
 */
struct hashmap *new_hashmap1(int size);


/**
 * @brief delete a hashmap object instance, free all the memory
 * 
 * @param map The hashmap object, created by 'new_hashmap()'
 */
void hashmap_delete(struct hashmap *map);


/**
 * @brief get an element with key from hashmap
 * 
 * @param map The hashmap object, created by 'new_hashmap()'
 * @param key Key to get the element
 * @return void* The elemet, the pointer must not be free
 */
void *hashmap_get(struct hashmap *map, char *key);


/**
 * @brief Put an element into hashmap, this is 
 * 
 * @param map The hashmap object, created by 'new_hashmap()'
 * @param key Key to get the element
 * @param value the object to be put into the hashmap
 * @param size the object size, if zero, treat object as string
 * @return 0 If success
 * @return 1 If error occur
 *
 * @code
 * struct hashmap *map = new_hashmap();
 * hashmap_put(map, "Hello", "World", 0);
 * printf("%s", hashmap_get(map, "Hello"));
 * hashmap_delete(map);
 * @endcode
 */
int hashmap_put(struct hashmap *map, const char *key, const void *value, size_t size);


/**
 * @brief Get element count stored in hashmap 
 * 
 * @param map The hashmap object, created by 'new_hashmap()'
 * @return int The element count
 */
int hashmap_size(struct hashmap *map);


/**
 * @brief Get all the keys of the elements, the return value need to be
 *          free by function 'hashmap_keys_free'
 * 
 * @param map The hashmap object, created by 'new_hashmap()'
 * @return char** The keys array, end with a NULL pointer.
 * 
 * @note The caller shouldn't modify the string in return value
 * @code
 * char **keys = hashmap_keys(map);
 * for (int i = 0; keys[i] != NULL; ++i)
 *     hashmap_get(map, keys[i]);
 * 
 * hashmap_keys_delete(keys);
 * hashmap_delete(map);
 * @endcode
 */
char **hashmap_keys(struct hashmap *map);


/**
 * @brief Free the key list generated by ‘hashmap_keys()‘
 * 
 * @param keys The key list generated by ‘hashmap_keys()‘
 */
void hashmap_keys_delete(char **keys);

#endif
