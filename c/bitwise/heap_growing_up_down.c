#include<stdio.h>
#include<stdlib.h>
void main()
{
int *p1=malloc(sizeof(int));
int *p2=malloc(sizeof(int));
printf("1st allocated p1; %p\n",&p1);
printf("2nd allocated p2; %p\n",&p2);

if(p2>p1)
printf("Heap is growing upward\n");
else
printf("Heap is growing downward\n");

free(p1);
free(p2);
}
