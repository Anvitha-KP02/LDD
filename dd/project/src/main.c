#include "list.h"
#include "queue.h"
#include "stack.h"
#include "utils.h"

#include <stdbool.h>
#include <stdio.h>

#define DEFAULT_CAPACITY 10

static void run_stack_menu(Stack *stack);
static void run_queue_menu(Queue *queue);
static void run_list_menu(LinkedList *list);

static bool get_menu_choice(const char *prompt, int min, int max, int *choice) {
    if (!read_int_in_range(prompt, min, max, choice)) {
        print_error("Invalid input. Enter a valid menu option.");
        log_message("WARN", "Invalid menu input.");
        return false;
    }
    return true;
}

int main(void) {
    Stack stack;
    Queue queue;
    LinkedList list;
    bool running = true;

    if (!stack_init(&stack, DEFAULT_CAPACITY)) {
        print_error("Failed to initialize stack.");
        return 1;
    }
    if (!queue_init(&queue, DEFAULT_CAPACITY)) {
        stack_destroy(&stack);
        print_error("Failed to initialize queue.");
        return 1;
    }
    list_init(&list);

    log_message("INFO", "Application started.");
    print_banner();

    while (running) {
        int choice;
        printf("\nMain Menu\n");
        printf("1. Stack\n");
        printf("2. Queue\n");
        printf("3. Linked List\n");
        printf("4. Exit\n");

        if (!get_menu_choice("Choose an option [1-4]: ", 1, 4, &choice)) {
            continue;
        }

        switch (choice) {
            case 1:
                run_stack_menu(&stack);
                break;
            case 2:
                run_queue_menu(&queue);
                break;
            case 3:
                run_list_menu(&list);
                break;
            case 4:
                running = false;
                break;
            default:
                break;
        }
    }

    stack_destroy(&stack);
    queue_destroy(&queue);
    list_destroy(&list);

    log_message("INFO", "Application exited cleanly.");
    print_success("Goodbye.");
    return 0;
}

static void run_stack_menu(Stack *stack) {
    bool active = true;

    while (active) {
        int choice;
        int value;
        printf("\nStack Menu\n");
        printf("1. Insert\n");
        printf("2. Delete\n");
        printf("3. Display\n");
        printf("4. Back\n");

        if (!get_menu_choice("Choose an option [1-4]: ", 1, 4, &choice)) {
            continue;
        }

        switch (choice) {
            case 1:
                if (!read_int("Enter value to insert: ", &value)) {
                    print_error("Invalid number.");
                    log_operation("stack", "insert", "invalid_input");
                    break;
                }
                if (!stack_insert(stack, value)) {
                    print_error("Stack overflow. Cannot insert.");
                    log_operation("stack", "insert", "overflow");
                } else {
                    print_success("Value inserted into stack.");
                    log_operation("stack", "insert", "success");
                }
                break;
            case 2:
                if (!stack_delete(stack, &value)) {
                    print_error("Stack underflow. Nothing to delete.");
                    log_operation("stack", "delete", "underflow");
                } else {
                    printf("Deleted value: %d\n", value);
                    log_operation("stack", "delete", "success");
                }
                break;
            case 3:
                stack_display(stack);
                log_operation("stack", "display", "success");
                break;
            case 4:
                active = false;
                break;
            default:
                break;
        }
    }
}

static void run_queue_menu(Queue *queue) {
    bool active = true;

    while (active) {
        int choice;
        int value;
        printf("\nQueue Menu\n");
        printf("1. Insert\n");
        printf("2. Delete\n");
        printf("3. Display\n");
        printf("4. Back\n");

        if (!get_menu_choice("Choose an option [1-4]: ", 1, 4, &choice)) {
            continue;
        }

        switch (choice) {
            case 1:
                if (!read_int("Enter value to insert: ", &value)) {
                    print_error("Invalid number.");
                    log_operation("queue", "insert", "invalid_input");
                    break;
                }
                if (!queue_insert(queue, value)) {
                    print_error("Queue overflow. Cannot insert.");
                    log_operation("queue", "insert", "overflow");
                } else {
                    print_success("Value inserted into queue.");
                    log_operation("queue", "insert", "success");
                }
                break;
            case 2:
                if (!queue_delete(queue, &value)) {
                    print_error("Queue underflow. Nothing to delete.");
                    log_operation("queue", "delete", "underflow");
                } else {
                    printf("Deleted value: %d\n", value);
                    log_operation("queue", "delete", "success");
                }
                break;
            case 3:
                queue_display(queue);
                log_operation("queue", "display", "success");
                break;
            case 4:
                active = false;
                break;
            default:
                break;
        }
    }
}

static void run_list_menu(LinkedList *list) {
    bool active = true;

    while (active) {
        int choice;
        int value;
        printf("\nLinked List Menu\n");
        printf("1. Insert\n");
        printf("2. Delete by value\n");
        printf("3. Display\n");
        printf("4. Back\n");

        if (!get_menu_choice("Choose an option [1-4]: ", 1, 4, &choice)) {
            continue;
        }

        switch (choice) {
            case 1:
                if (!read_int("Enter value to insert: ", &value)) {
                    print_error("Invalid number.");
                    log_operation("list", "insert", "invalid_input");
                    break;
                }
                if (!list_insert(list, value)) {
                    print_error("Insert failed. Possible memory issue.");
                    log_operation("list", "insert", "failed");
                } else {
                    print_success("Value inserted into linked list.");
                    log_operation("list", "insert", "success");
                }
                break;
            case 2:
                if (!read_int("Enter value to delete: ", &value)) {
                    print_error("Invalid number.");
                    log_operation("list", "delete", "invalid_input");
                    break;
                }
                if (!list_delete(list, value)) {
                    print_error("Value not found in linked list.");
                    log_operation("list", "delete", "not_found");
                } else {
                    print_success("Value deleted from linked list.");
                    log_operation("list", "delete", "success");
                }
                break;
            case 3:
                list_display(list);
                log_operation("list", "display", "success");
                break;
            case 4:
                active = false;
                break;
            default:
                break;
        }
    }
}
