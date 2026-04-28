#include "task_manager.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void safe_copy(char *dst, size_t dst_len, const char *src) {
    if (dst_len == 0) {
        return;
    }
    strncpy(dst, src, dst_len - 1);
    dst[dst_len - 1] = '\0';
}

void init_task_list(TaskList *list) {
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    list->next_id = 1;
}

bool add_task(TaskList *list, const char *title, const char *assignee, int priority, int *out_id) {
    Task *new_node = (Task *)calloc(1, sizeof(Task));
    if (new_node == NULL) {
        return false;
    }

    new_node->id = list->next_id++;
    safe_copy(new_node->title, sizeof(new_node->title), title);
    safe_copy(new_node->assignee, sizeof(new_node->assignee), assignee);
    safe_copy(new_node->status, sizeof(new_node->status), "TODO");
    new_node->priority = priority;
    new_node->next = NULL;

    if (list->tail == NULL) {
        list->head = list->tail = new_node;
    } else {
        list->tail->next = new_node;
        list->tail = new_node;
    }

    list->count++;
    if (out_id != NULL) {
        *out_id = new_node->id;
    }
    return true;
}

Task *find_task_by_id(TaskList *list, int id) {
    Task *cur = list->head;
    while (cur != NULL) {
        if (cur->id == id) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

bool update_task_status(TaskList *list, int id, const char *new_status) {
    Task *task = find_task_by_id(list, id);
    if (task == NULL) {
        return false;
    }
    safe_copy(task->status, sizeof(task->status), new_status);
    return true;
}

bool remove_task(TaskList *list, int id) {
    Task *prev = NULL;
    Task *cur = list->head;

    while (cur != NULL) {
        if (cur->id == id) {
            if (prev == NULL) {
                list->head = cur->next;
            } else {
                prev->next = cur->next;
            }
            if (cur == list->tail) {
                list->tail = prev;
            }
            free(cur);
            list->count--;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

void print_all_tasks(const TaskList *list) {
    const Task *cur = list->head;
    if (cur == NULL) {
        puts("No tasks found.");
        return;
    }

    puts("ID | Priority | Status        | Assignee                        | Title");
    puts("--------------------------------------------------------------------------");
    while (cur != NULL) {
        printf("%-2d | %-8d | %-13s | %-31s | %s\n",
               cur->id, cur->priority, cur->status, cur->assignee, cur->title);
        cur = cur->next;
    }
}

void print_tasks_by_assignee(const TaskList *list, const char *assignee) {
    const Task *cur = list->head;
    bool found = false;
    while (cur != NULL) {
        if (strcmp(cur->assignee, assignee) == 0) {
            if (!found) {
                puts("ID | Priority | Status        | Title");
                puts("--------------------------------------");
            }
            found = true;
            printf("%-2d | %-8d | %-13s | %s\n", cur->id, cur->priority, cur->status, cur->title);
        }
        cur = cur->next;
    }

    if (!found) {
        puts("No tasks found for this assignee.");
    }
}

void free_task_list(TaskList *list) {
    Task *cur = list->head;
    while (cur != NULL) {
        Task *next = cur->next;
        free(cur);
        cur = next;
    }
    init_task_list(list);
}

void init_queue(TaskQueue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->count = 0;
}

bool enqueue_task(TaskQueue *queue, int task_id) {
    QueueNode *node = (QueueNode *)calloc(1, sizeof(QueueNode));
    if (node == NULL) {
        return false;
    }
    node->task_id = task_id;
    node->next = NULL;

    if (queue->rear == NULL) {
        queue->front = queue->rear = node;
    } else {
        queue->rear->next = node;
        queue->rear = node;
    }
    queue->count++;
    return true;
}

bool dequeue_task(TaskQueue *queue, int *out_task_id) {
    if (queue->front == NULL) {
        return false;
    }
    QueueNode *node = queue->front;
    if (out_task_id != NULL) {
        *out_task_id = node->task_id;
    }
    queue->front = node->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(node);
    queue->count--;
    return true;
}

bool remove_task_from_queue(TaskQueue *queue, int task_id) {
    QueueNode *prev = NULL;
    QueueNode *cur = queue->front;
    while (cur != NULL) {
        if (cur->task_id == task_id) {
            if (prev == NULL) {
                queue->front = cur->next;
            } else {
                prev->next = cur->next;
            }
            if (cur == queue->rear) {
                queue->rear = prev;
            }
            free(cur);
            queue->count--;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

void print_queue(const TaskQueue *queue) {
    const QueueNode *cur = queue->front;
    if (cur == NULL) {
        puts("Execution queue is empty.");
        return;
    }
    puts("Queued task IDs (FIFO):");
    while (cur != NULL) {
        printf("%d%s", cur->task_id, (cur->next == NULL) ? "\n" : " -> ");
        cur = cur->next;
    }
}

void free_queue(TaskQueue *queue) {
    int ignored = 0;
    while (dequeue_task(queue, &ignored)) {
    }
}
