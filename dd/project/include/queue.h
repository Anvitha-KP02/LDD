#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int *data;
    size_t capacity;
    size_t size;
    int front;
    int rear;
} Queue;

bool queue_init(Queue *queue, size_t capacity);
void queue_destroy(Queue *queue);

bool queue_insert(Queue *queue, int value);
bool queue_delete(Queue *queue, int *deleted_value);
void queue_display(const Queue *queue);

bool queue_is_empty(const Queue *queue);
bool queue_is_full(const Queue *queue);
size_t queue_size(const Queue *queue);

#endif
