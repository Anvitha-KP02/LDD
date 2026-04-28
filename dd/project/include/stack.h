#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int *data;
    size_t capacity;
    int top;
} Stack;

bool stack_init(Stack *stack, size_t capacity);
void stack_destroy(Stack *stack);

bool stack_insert(Stack *stack, int value);
bool stack_delete(Stack *stack, int *deleted_value);
void stack_display(const Stack *stack);

bool stack_is_empty(const Stack *stack);
bool stack_is_full(const Stack *stack);
size_t stack_size(const Stack *stack);

#endif
