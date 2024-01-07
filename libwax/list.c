#include <wax/list.h>


void list_head_init(struct list_head *head)
{
    head->next = NULL;
    head->prev = NULL;
}


void list_add(struct list_head *node, struct list_head *target)
{
    struct list_head *next = node->next;

    node->next = target;
    target->prev = node;
    target->next = next;

    if (next != NULL)
        next->prev = target;
}


void list_add_tail(struct list_head *node, struct list_head *target)
{
    struct list_head *tail = node;
    while (tail->next != NULL)
        tail = node->next;

    list_add(tail, target);
}


void list_del(struct list_head *node)
{
    struct list_head *next = node->next;
    struct list_head *prev = node->prev;
    
    if (next !=  NULL)
        next->prev = prev;

    if (prev != NULL)
        prev->next = next;
    
    node->next = NULL;
    node->prev = NULL;
}


void list_move(struct list_head *node, struct list_head *target_node)
{
    list_del(node);
    list_add(target_node, node);
}


void list_move_tail(struct list_head *node)
{
    list_add_tail(node, node);
    list_del(node);
}


bool inline list_is_empty(struct list_head *head)
{
    if (head == NULL)
        return true;
    return head->next == NULL;
}
