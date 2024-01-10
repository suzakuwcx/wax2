#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <wax/hashmap.h>


struct hashmap {
    int max_size;
    struct list_head **slot;
};

struct linklist {
    char *key;
    void *value;
    struct list_head head;
};


static void *cpy_wrapper(const void *src, size_t size)
{
    void *ptr;

    if (size == 0) {
        return strdup(src);
    }
    else {
        ptr = calloc(1, size);
        memcpy(ptr, src, size);
        return ptr;
    }
}


static struct linklist * new_linklist(const char *key, const void *value, size_t size)
{
    struct linklist *list;
    list = calloc(1, sizeof(struct linklist));
    list->key = strdup(key);
    list->value = cpy_wrapper(value, size);
    return list;
}


static void linklist_delete(struct list_head *head)
{
    if (head == NULL)
        return;

    struct list_head *next = head->next;
    struct linklist *list = list_entry(head, struct linklist, head);

    free(list->key);
    free(list->value);
    free(list);

    linklist_delete(next);
}


static unsigned int hash(const char *key) {
    unsigned int hash = 0;
    int len = strlen(key);
    unsigned int *buff = (unsigned int *)key;
    int pow_i = 1;

    /*
     * assume char array is a four byte integer array
     */
    while (len >=4) {
        hash += *buff;
        ++buff;
        len -= 4;
    }

    /*
     * append the last byte that not long enough:
     * buff: (unsgined int *) '1' '2' '\0' 'a' -> 'a' is random value in memory
     * but what we need is:
     * buff: (unsigned int *) '1' '2' '\0' '\0'
     *
     * so, the hash is:
     * 1 * (256^0) + 2 * (256 ^ 1)
     */ 

    for (int i = 0; i < len; ++i) {
        for (int j = 0; j < i; ++j)
            pow_i *= 256;

        hash += ((char *)buff)[i] * pow_i;
        pow_i = 1;
    }

    return hash;
}


struct hashmap *new_hashmap1(int size)
{
    struct hashmap *map = malloc(sizeof(struct hashmap));
    map->max_size = size;

    map->slot = (struct list_head **) calloc(size, sizeof(struct list_head));
    return map;
}


struct hashmap *new_hashmap()
{
    return new_hashmap1(16);
}


void hashmap_delete(struct hashmap *map)
{
    struct list_head *head;
    struct linklist *list;

    for (int i = 0; i < map->max_size; ++i) {
        head = (map->slot)[i];
        linklist_delete(head);
    }

    free(map->slot);
    free(map);
}


int hashmap_put(struct hashmap *map, const char *key, const void *value, size_t size)
{
    int hs = hash(key) % map->max_size;

    struct list_head *head = (map->slot)[hs];
    struct linklist *list;

    /* no element */ 
    if (head == NULL) {
        list = new_linklist(key, value, size);
        (map->slot)[hs] = &(list->head);
        return 0;
    }

    while (head->next != NULL) {
        list = list_entry(head, struct linklist, head);
        // find an element with the same key
        if (strcmp(list->key, key) == 0) {
            free(list->value);
            list->value = cpy_wrapper(value, size);
            return 0;
        }

        head = head->next;
    }

    /* no matching key found, insert a new element at the end */

    list = new_linklist(key, value, size);
    list_add(head, &(list->head));

    return 0;
}


void *hashmap_get(struct hashmap *map, char *key)
{
    int hs = hash(key) % map->max_size;

    struct list_head *head = (map->slot)[hs];
    struct linklist *list;

    while (head != NULL) {
        list = list_entry(head, struct linklist, head);
        if (strcmp(list->key, key) == 0)
            return list->value;

        head = head->next;
    }

    return NULL;
}


int hashmap_size(struct hashmap *map)
{
    struct list_head *head;
    int count = 0;

    for (int i = 0; i < map->max_size; ++i) {
        head = (map->slot)[i];
        while (head != NULL) {
            ++count;
            head = head->next;
        }
    }

    return count;
}


char **hashmap_keys(struct hashmap *map)
{
    struct list_head *head;
    struct linklist *list;

    int len = hashmap_size(map);
    char **keys = calloc(len + 1, sizeof(char *));
    int j = 0;

    for (int i = 0; i < map->max_size; ++i) {
        head = map->slot[i];
        while (head != NULL) {
            list = list_entry(head, struct linklist, head);
            keys[j] = strdup(list->key);
            ++j;
            head = head->next;
        }
    }

    return keys;
}


void hashmap_keys_delete(char **keys)
{
    for (int i = 0; keys[i] != NULL; ++i)
        free(keys[i]);

    free(keys);
}
