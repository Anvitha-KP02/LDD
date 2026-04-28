#include "task_manager.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_linked_list(void) {
    TaskList list;
    int id1 = 0;
    int id2 = 0;
    Task *task = NULL;

    init_task_list(&list);
    assert(add_task(&list, "Design API", "Anvitha", 3, &id1));
    assert(add_task(&list, "Write tests", "Mira", 2, &id2));
    assert(id1 == 1);
    assert(id2 == 2);
    assert(list.count == 2);

    task = find_task_by_id(&list, id2);
    assert(task != NULL);
    assert(strcmp(task->title, "Write tests") == 0);

    assert(update_task_status(&list, id1, "IN_PROGRESS"));
    task = find_task_by_id(&list, id1);
    assert(task != NULL);
    assert(strcmp(task->status, "IN_PROGRESS") == 0);

    assert(remove_task(&list, id1));
    assert(find_task_by_id(&list, id1) == NULL);
    assert(list.count == 1);

    free_task_list(&list);
}

static void test_queue(void) {
    TaskQueue queue;
    int out = 0;

    init_queue(&queue);
    assert(enqueue_task(&queue, 10));
    assert(enqueue_task(&queue, 20));
    assert(queue.count == 2);

    assert(dequeue_task(&queue, &out));
    assert(out == 10);
    assert(queue.count == 1);

    assert(remove_task_from_queue(&queue, 20));
    assert(queue.count == 0);
    assert(!dequeue_task(&queue, &out));

    free_queue(&queue);
}

int main(void) {
    test_linked_list();
    test_queue();
    puts("All DS tests passed.");
    return 0;
}
