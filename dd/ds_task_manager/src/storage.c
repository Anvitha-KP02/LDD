#include "task_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool save_tasks_to_file(const TaskList *list, const char *file_path) {
    const Task *cur = list->head;
    FILE *fp = fopen(file_path, "w");
    if (fp == NULL) {
        return false;
    }

    while (cur != NULL) {
        fprintf(fp, "%d|%s|%s|%d|%s\n",
                cur->id, cur->title, cur->assignee, cur->priority, cur->status);
        cur = cur->next;
    }
    fclose(fp);
    return true;
}

bool load_tasks_from_file(TaskList *list, const char *file_path) {
    char line[512];
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        return false;
    }

    free_task_list(list);
    init_task_list(list);

    while (fgets(line, sizeof(line), fp) != NULL) {
        int id = 0;
        int priority = 0;
        char title[TITLE_MAX_LEN + 1] = {0};
        char assignee[ASSIGNEE_MAX_LEN + 1] = {0};
        char status[STATUS_MAX_LEN + 1] = {0};
        Task *created = NULL;

        if (sscanf(line, "%d|%63[^|]|%31[^|]|%d|%15[^\n]",
                   &id, title, assignee, &priority, status) != 5) {
            continue;
        }

        if (!add_task(list, title, assignee, priority, NULL)) {
            fclose(fp);
            return false;
        }

        created = list->tail;
        created->id = id;
        strncpy(created->status, status, sizeof(created->status) - 1);
        created->status[sizeof(created->status) - 1] = '\0';
        if (id >= list->next_id) {
            list->next_id = id + 1;
        }
    }

    fclose(fp);
    return true;
}
