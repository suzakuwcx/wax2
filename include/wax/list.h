#ifndef _LIST_H_
#define _LIST_H_ 1

/*
 * Doubly Linked List
 *
 * head: the head node of the link list
 * node: current node of the link list
 * target: target node to be inserted
 * next: next node of the current node
 * prev: previous node of the current node
 */

#include <stddef.h>
#include <stdbool.h>

#define container_of(ptr, type, member) ({                  \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};


/**
 * @brief Init a list_head object
 * 
 * @param head The member store in a structure, it must not create by 'malloc'
 * @code
 * struct foo {
 *     int value;
 *     struct list_head head;
 * } bar1, bar2;
 * list_head_init(&(bar1.head));
 * @endcode
 */
void list_head_init(struct list_head *node);


/**
 * @brief Add node 'target' after the 'node'
 * 
 *     $ -----> $          $
 *  (node)    (next)    (target)
 *
 *     $ -----> $ -----> $
 *  (node)   (target)  (next)
 *
 * @param node The current node
 * @param target The node to be insert after the 'node'
 *
 * @code
 * struct list_head *head = &(bar1.head);
 * list_add(head, &(bar2.head));
 * @endcode
 */
void list_add(struct list_head *node, struct list_head *target);


/**
 * @brief Add node 'target' at the end of the link list 'node'
 * 
 *     $ -----> $ ---- ...... ---> $         $
 *  (node)     (next)            (end)    (target)
 *
 *     $ -----> $ ---- ...... ---> $ ----->  $
 *  (node)     (next)            (end)    (target)
 *
 * @param node The current node
 * @param target The node to be insert at the end of the list 'node'
 */
void list_add_tail(struct list_head *node, struct list_head *target);


/**
 * @brief Remove 'node' from it's list
 * 
 *     $ -----> $ -----> $
 *  (prev)   (node)    (next)
 *
 *     $ -----> $        $
 *  (prev)   (next)    (node)
 *
 * @param node The node to be remove
 */
void list_del(struct list_head *node);


/**
 * @brief Move 'node' after the 'target_node'
 *
 *     $ -----> $ ---- .. --> $ -- .... ---> $
 *  (prev)     (node)    (target_node)    (end)
 *
 *     $ ---- .. --> $ -----> $ -- .... ---> $
 *  (prev)     (target_node)(node)         (end)
 * 
 * @param node The node to be remove
 * @param target_node The target node that the 'node' will move after
 */
void list_move(struct list_head *node, struct list_head *target_node);


/**
 * @brief Move 'node' to the tail of its list
 *
 *     $ -----> $ ---- .. --> $
 *  (prev)     (node)       (end)
 *
 *     $ --- .. --> $ -----> $
 *  (prev)        (end)    (node)
 *
 * @param node 
 */
void list_move_tail(struct list_head *node);


/**
 * @brief Check if list empty or not
 * 
 * @param head The head of the list
 * @return true The list is empty
 *         false The list is no empty
 *       
 */
bool list_is_empty(struct list_head *head);

#define list_entry(head, type, member) \
    container_of(head, type, member)

#endif /* _LIST_H_ */
