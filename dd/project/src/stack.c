#include "stack.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

bool stack_init(Stack *stack, size_t capacity) {
    if (stack == NULL || capacity == 0) {
        return false;
    }

    stack->data = (int *)malloc(capacity * sizeof(int));
    if (stack->data == NULL) {
        return false;
    }

    stack->capacity = capacity;
    stack->top = -1;
    return true;
}

void stack_destroy(Stack *stack) {
    if (stack == NULL) {
        return;
    }

    free(stack->data);
    stack->data = NULL;
    stack->capacity = 0;
    stack->top = -1;
}

bool stack_insert(Stack *stack, int value) {
    if (stack == NULL || stack->data == NULL) {
        return false;
    }
    if (stack_is_full(stack)) {
        DEBUG_PRINT("Stack overflow while inserting value=%d", value);
        return false;
    }

    stack->data[++stack->top] = value;
    DEBUG_PRINT("Stack insert value=%d, top=%d", value, stack->top);
    return true;
}

bool stack_delete(Stack *stack, int *deleted_value) {
    if (stack == NULL || stack->data == NULL || deleted_value == NULL) {
        return false;
    }
    if (stack_is_empty(stack)) {
        DEBUG_PRINT("Stack underflow on delete");
        return false;
    }

    *deleted_value = stack->data[stack->top--];
    DEBUG_PRINT("Stack delete value=%d, new_top=%d", *deleted_value, stack->top);
    return true;
}

void stack_display(const Stack *stack) {
    int i;
    if (stack == NULL || stack->data == NULL) {
        print_error("Stack is not initialized.");
        return;
    }
    if (stack_is_empty(stack)) {
        print_info("Stack is empty.");
        return;
    }

    printf("Stack (top -> bottom): ");
    for (i = stack->top; i >= 0; --i) {
        printf("%d", stack->data[i]);
        if (i > 0) {
            printf(" ");
        }
    }
    printf("\n");
}

bool stack_is_empty(const Stack *stack) {
    return stack == NULL || stack->top < 0;
}

bool stack_is_full(const Stack *stack) {
    if (stack == NULL) {
        return false;
    }
    return (size_t)(stack->top + 1) >= stack->capacity;
}

size_t stack_size(const Stack *stack) {
    if (stack == NULL || stack->top < 0) {
        return 0;
    }
    return (size_t)(stack->top + 1);
}
