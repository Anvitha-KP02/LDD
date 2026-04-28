#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct ListNode {
    int value;
    struct ListNode *next;
} ListNode;

typedef struct {
    ListNode *head;
    size_t size;
} LinkedList;

void list_init(LinkedList *list);
void list_destroy(LinkedList *list);

bool list_insert(LinkedList *list, int value);
bool list_delete(LinkedList *list, int value);
void list_display(const LinkedList *list);

size_t list_size(const LinkedList *list);
bool list_is_empty(const LinkedList *list);

#endif
