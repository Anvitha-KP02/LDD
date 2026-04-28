#include "cli.h"
#include "logger.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

static int read_line(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) {
        return -1;
    }
    trim_newline(buffer);
    return 0;
}

static int read_int_range(const char *prompt, int min, int max, int *out) {
    char input[64];
    char *end = NULL;
    long value = 0;

    for (;;) {
        printf("%s", prompt);
        if (read_line(input, sizeof(input)) != 0) {
            return -1;
        }
        if (input[0] == '\0') {
            puts("Input cannot be empty.");
            continue;
        }
        value = strtol(input, &end, 10);
        if (*end != '\0') {
            puts("Please enter a valid number.");
            continue;
        }
        if (value < min || value > max) {
            printf("Value must be between %d and %d.\n", min, max);
            continue;
        }
        *out = (int)value;
        return 0;
    }
}

static int read_non_empty_string(const char *prompt, char *out, size_t out_size) {
    for (;;) {
        printf("%s", prompt);
        if (read_line(out, out_size) != 0) {
            return -1;
        }
        if (out[0] == '\0') {
            puts("Input cannot be empty.");
            continue;
        }
        return 0;
    }
}

static int is_valid_status(const char *s) {
    return strcmp(s, "TODO") == 0 || strcmp(s, "IN_PROGRESS") == 0 || strcmp(s, "DONE") == 0;
}

static void print_menu(void) {
    puts("\n========== Task Manager ==========");
    puts("1) Add task");
    puts("2) List all tasks");
    puts("3) Update task status");
    puts("4) Delete task");
    puts("5) List tasks by assignee");
    puts("6) Enqueue task for execution");
    puts("7) Dequeue next task");
    puts("8) Show execution queue");
    puts("9) Save tasks");
    puts("10) Load tasks");
    puts("0) Exit");
}

void run_cli(TaskList *tasks, TaskQueue *queue, const char *data_file) {
    int running = 1;

    while (running) {
        int choice = -1;
        print_menu();
        if (read_int_range("Choose an option: ", 0, 10, &choice) != 0) {
            puts("\nInput stream closed. Exiting.");
            break;
        }

        switch (choice) {
            case 1: {
                int priority = 0;
                int task_id = -1;
                char title[TITLE_MAX_LEN + 1];
                char assignee[ASSIGNEE_MAX_LEN + 1];

                if (read_non_empty_string("Title: ", title, sizeof(title)) != 0 ||
                    read_non_empty_string("Assignee: ", assignee, sizeof(assignee)) != 0 ||
                    read_int_range("Priority (1-5): ", 1, 5, &priority) != 0) {
                    puts("Unable to read input.");
                    break;
                }

                if (add_task(tasks, title, assignee, priority, &task_id)) {
                    printf("Task created with ID %d.\n", task_id);
                    log_message(LOG_LEVEL_INFO, "task_created id=%d title='%s' assignee='%s'", task_id, title, assignee);
                } else {
                    puts("Failed to create task.");
                    log_message(LOG_LEVEL_ERROR, "task_create_failed");
                }
                break;
            }
            case 2:
                print_all_tasks(tasks);
                break;
            case 3: {
                int id = 0;
                char status[STATUS_MAX_LEN + 1];

                if (read_int_range("Task ID: ", 1, 1000000, &id) != 0 ||
                    read_non_empty_string("New status (TODO/IN_PROGRESS/DONE): ", status, sizeof(status)) != 0) {
                    puts("Unable to read input.");
                    break;
                }
                if (!is_valid_status(status)) {
                    puts("Invalid status.");
                    break;
                }
                if (update_task_status(tasks, id, status)) {
                    puts("Status updated.");
                    log_message(LOG_LEVEL_INFO, "task_status_updated id=%d status='%s'", id, status);
                } else {
                    puts("Task not found.");
                    log_message(LOG_LEVEL_WARN, "task_status_update_not_found id=%d", id);
                }
                break;
            }
            case 4: {
                int id = 0;
                if (read_int_range("Task ID to delete: ", 1, 1000000, &id) != 0) {
                    puts("Unable to read input.");
                    break;
                }
                if (remove_task(tasks, id)) {
                    remove_task_from_queue(queue, id);
                    puts("Task deleted.");
                    log_message(LOG_LEVEL_INFO, "task_deleted id=%d", id);
                } else {
                    puts("Task not found.");
                    log_message(LOG_LEVEL_WARN, "task_delete_not_found id=%d", id);
                }
                break;
            }
            case 5: {
                char assignee[ASSIGNEE_MAX_LEN + 1];
                if (read_non_empty_string("Assignee name: ", assignee, sizeof(assignee)) != 0) {
                    puts("Unable to read input.");
                    break;
                }
                print_tasks_by_assignee(tasks, assignee);
                break;
            }
            case 6: {
                int id = 0;
                if (read_int_range("Task ID to enqueue: ", 1, 1000000, &id) != 0) {
                    puts("Unable to read input.");
                    break;
                }
                if (find_task_by_id(tasks, id) == NULL) {
                    puts("Task not found. Create task first.");
                    break;
                }
                if (enqueue_task(queue, id)) {
                    puts("Task enqueued.");
                    log_message(LOG_LEVEL_INFO, "task_enqueued id=%d", id);
                } else {
                    puts("Failed to enqueue task.");
                    log_message(LOG_LEVEL_ERROR, "task_enqueue_failed id=%d", id);
                }
                break;
            }
            case 7: {
                int id = 0;
                if (dequeue_task(queue, &id)) {
                    printf("Dequeued task ID: %d\n", id);
                    log_message(LOG_LEVEL_INFO, "task_dequeued id=%d", id);
                } else {
                    puts("Queue is empty.");
                    log_message(LOG_LEVEL_WARN, "task_dequeue_empty");
                }
                break;
            }
            case 8:
                print_queue(queue);
                break;
            case 9:
                if (save_tasks_to_file(tasks, data_file)) {
                    puts("Tasks saved.");
                    log_message(LOG_LEVEL_INFO, "tasks_saved file='%s' count=%zu", data_file, tasks->count);
                } else {
                    puts("Save failed.");
                    log_message(LOG_LEVEL_ERROR, "tasks_save_failed file='%s'", data_file);
                }
                break;
            case 10:
                if (load_tasks_from_file(tasks, data_file)) {
                    puts("Tasks loaded.");
                    log_message(LOG_LEVEL_INFO, "tasks_loaded file='%s' count=%zu", data_file, tasks->count);
                } else {
                    puts("Load failed (file may not exist).");
                    log_message(LOG_LEVEL_WARN, "tasks_load_failed file='%s'", data_file);
                }
                break;
            case 0:
                running = 0;
                break;
            default:
                puts("Unknown option.");
                break;
        }
    }
}
