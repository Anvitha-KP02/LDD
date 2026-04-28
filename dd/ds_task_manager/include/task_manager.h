#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <stdbool.h>
#include <stddef.h>

#define TITLE_MAX_LEN 63
#define ASSIGNEE_MAX_LEN 31
#define STATUS_MAX_LEN 15

typedef struct Task {
    int id;
    char title[TITLE_MAX_LEN + 1];
    char assignee[ASSIGNEE_MAX_LEN + 1];
    int priority;
    char status[STATUS_MAX_LEN + 1];
    struct Task *next;
} Task;

typedef struct {
    Task *head;
    Task *tail;
    size_t count;
    int next_id;
} TaskList;

typedef struct QueueNode {
    int task_id;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    size_t count;
} TaskQueue;

void init_task_list(TaskList *list);
bool add_task(TaskList *list, const char *title, const char *assignee, int priority, int *out_id);
Task *find_task_by_id(TaskList *list, int id);
bool update_task_status(TaskList *list, int id, const char *new_status);
bool remove_task(TaskList *list, int id);
void print_all_tasks(const TaskList *list);
void print_tasks_by_assignee(const TaskList *list, const char *assignee);
void free_task_list(TaskList *list);

void init_queue(TaskQueue *queue);
bool enqueue_task(TaskQueue *queue, int task_id);
bool dequeue_task(TaskQueue *queue, int *out_task_id);
bool remove_task_from_queue(TaskQueue *queue, int task_id);
void print_queue(const TaskQueue *queue);
void free_queue(TaskQueue *queue);

bool save_tasks_to_file(const TaskList *list, const char *file_path);
bool load_tasks_from_file(TaskList *list, const char *file_path);

#endif
