#include<stdio.h>
void check(int *main_addr)
{
int local;
printf("Address in main: %p\n",(void*)main_addr);
printf("Address in function: %p\n",(void*)&local);
if(&local > main_addr)
printf("Stack grows upward\n");
else
printf("Stack grows downward\n");
}

void main()
{
int x;
printf("Address of x in main: %p\n",(void*)&x);
check(&x);
}

/*#include <stdio.h>

int main()
{
    char x, y;

    printf("Address of x: %p\n", (void*)&x);
    printf("Address of y: %p\n", (void*)&y);

    if((char*)&y > (char*)&x)
        printf("Stack grows downward\n");
    else
        printf("Stack grows upward\n");

    return 0;
}*/


// To check the stack growth, i compare the address of two variables.
// (char*)&y - (char*)&x
// If the address of 2nd/later variable is greater, then the stack is growing downwards
