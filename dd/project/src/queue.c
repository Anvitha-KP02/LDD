#include "queue.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

bool queue_init(Queue *queue, size_t capacity) {
    if (queue == NULL || capacity == 0) {
        return false;
    }

    queue->data = (int *)malloc(capacity * sizeof(int));
    if (queue->data == NULL) {
        return false;
    }

    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
    return true;
}

void queue_destroy(Queue *queue) {
    if (queue == NULL) {
        return;
    }

    free(queue->data);
    queue->data = NULL;
    queue->capacity = 0;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
}

bool queue_insert(Queue *queue, int value) {
    if (queue == NULL || queue->data == NULL) {
        return false;
    }
    if (queue_is_full(queue)) {
        DEBUG_PRINT("Queue overflow while inserting value=%d", value);
        return false;
    }

    queue->rear = (queue->rear + 1) % (int)queue->capacity;
    queue->data[queue->rear] = value;
    queue->size++;
    DEBUG_PRINT("Queue insert value=%d, front=%d, rear=%d, size=%zu",
                value, queue->front, queue->rear, queue->size);
    return true;
}

bool queue_delete(Queue *queue, int *deleted_value) {
    if (queue == NULL || queue->data == NULL || deleted_value == NULL) {
        return false;
    }
    if (queue_is_empty(queue)) {
        DEBUG_PRINT("Queue underflow on delete");
        return false;
    }

    *deleted_value = queue->data[queue->front];
    queue->front = (queue->front + 1) % (int)queue->capacity;
    queue->size--;
    DEBUG_PRINT("Queue delete value=%d, front=%d, rear=%d, size=%zu",
                *deleted_value, queue->front, queue->rear, queue->size);
    return true;
}

void queue_display(const Queue *queue) {
    size_t i;
    if (queue == NULL || queue->data == NULL) {
        print_error("Queue is not initialized.");
        return;
    }
    if (queue_is_empty(queue)) {
        print_info("Queue is empty.");
        return;
    }

    printf("Queue (front -> rear): ");
    for (i = 0; i < queue->size; ++i) {
        int index = (queue->front + (int)i) % (int)queue->capacity;
        printf("%d", queue->data[index]);
        if (i + 1 < queue->size) {
            printf(" ");
        }
    }
    printf("\n");
}

bool queue_is_empty(const Queue *queue) {
    return queue == NULL || queue->size == 0;
}

bool queue_is_full(const Queue *queue) {
    return queue != NULL && queue->size == queue->capacity;
}

size_t queue_size(const Queue *queue) {
    if (queue == NULL) {
        return 0;
    }
    return queue->size;
}
