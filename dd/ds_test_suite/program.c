#include <stdio.h>
#include <stdlib.h>

#define MAX 5

static int stack[MAX];
static int top = -1;

static void print_menu(void) {
    printf("\n=== STACK MENU ===\n");
    printf("1. Insert\n");
    printf("2. Delete\n");
    printf("3. Display\n");
    printf("4. Exit\n");
    printf("Enter choice: ");
}

static void insert_element(void) {
    int value;

    if (top == MAX - 1) {
        printf("Overflow: stack is full.\n");
        return;
    }

    printf("Enter value to insert: ");
    if (scanf("%d", &value) != 1) {
        printf("Invalid input.\n");
        return;
    }

    top++;
    stack[top] = value;
    printf("Inserted: %d\n", value);
}

static void delete_element(void) {
    int deleted_value;

    if (top == -1) {
        printf("Underflow: stack is empty.\n");
        return;
    }

    deleted_value = stack[top];
    top--;
    printf("Deleted: %d\n", deleted_value);
}

static void display_stack(void) {
    int i;

    if (top == -1) {
        printf("Stack is empty.\n");
        return;
    }

    printf("Stack elements (top to bottom): ");
    for (i = top; i >= 0; i--) {
        printf("%d", stack[i]);
        if (i != 0) {
            printf(" ");
        }
    }
    printf("\n");
}

int main(void) {
    int choice;

    while (1) {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Exiting.\n");
            break;
        }

        switch (choice) {
            case 1:
                insert_element();
                break;
            case 2:
                delete_element();
                break;
            case 3:
                display_stack();
                break;
            case 4:
                printf("Exiting program.\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }

    return 0;
}
