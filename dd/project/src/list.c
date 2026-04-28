#include "list.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void list_init(LinkedList *list) {
    if (list == NULL) {
        return;
    }
    list->head = NULL;
    list->size = 0;
}

void list_destroy(LinkedList *list) {
    ListNode *current;
    if (list == NULL) {
        return;
    }

    current = list->head;
    while (current != NULL) {
        ListNode *next = current->next;
        free(current);
        current = next;
    }

    list->head = NULL;
    list->size = 0;
}

bool list_insert(LinkedList *list, int value) {
    ListNode *node;
    ListNode *cursor;
    if (list == NULL) {
        return false;
    }

    node = (ListNode *)malloc(sizeof(ListNode));
    if (node == NULL) {
        return false;
    }
    node->value = value;
    node->next = NULL;

    if (list->head == NULL) {
        list->head = node;
    } else {
        cursor = list->head;
        while (cursor->next != NULL) {
            cursor = cursor->next;
        }
        cursor->next = node;
    }

    list->size++;
    DEBUG_PRINT("List insert value=%d, size=%zu", value, list->size);
    return true;
}

bool list_delete(LinkedList *list, int value) {
    ListNode *current;
    ListNode *previous;
    if (list == NULL || list->head == NULL) {
        DEBUG_PRINT("List delete failed, list empty");
        return false;
    }

    current = list->head;
    previous = NULL;
    while (current != NULL) {
        if (current->value == value) {
            if (previous == NULL) {
                list->head = current->next;
            } else {
                previous->next = current->next;
            }
            free(current);
            list->size--;
            DEBUG_PRINT("List delete value=%d, size=%zu", value, list->size);
            return true;
        }
        previous = current;
        current = current->next;
    }

    DEBUG_PRINT("List delete value=%d not found", value);
    return false;
}

void list_display(const LinkedList *list) {
    ListNode *current;
    if (list == NULL) {
        print_error("Linked list is not initialized.");
        return;
    }
    if (list_is_empty(list)) {
        print_info("Linked List is empty.");
        return;
    }

    printf("Linked List (head -> tail): ");
    current = list->head;
    while (current != NULL) {
        printf("%d", current->value);
        if (current->next != NULL) {
            printf(" -> ");
        }
        current = current->next;
    }
    printf("\n");
}

size_t list_size(const LinkedList *list) {
    if (list == NULL) {
        return 0;
    }
    return list->size;
}

bool list_is_empty(const LinkedList *list) {
    return list == NULL || list->head == NULL;
}
