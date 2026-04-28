#include "cli.h"
#include "logger.h"
#include "task_manager.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    TaskList tasks;
    TaskQueue queue;
    const char *data_file = "data/tasks.db";

    if (argc > 1) {
        data_file = argv[1];
    }

    init_task_list(&tasks);
    init_queue(&queue);

    if (logger_init("logs/task_manager.log") != 0) {
        fprintf(stderr, "Warning: failed to initialize logger.\n");
    }

    log_message(LOG_LEVEL_INFO, "app_started data_file='%s'", data_file);
    run_cli(&tasks, &queue, data_file);
    log_message(LOG_LEVEL_INFO, "app_shutdown tasks=%zu queue=%zu", tasks.count, queue.count);

    free_task_list(&tasks);
    free_queue(&queue);
    logger_close();
    return EXIT_SUCCESS;
}
