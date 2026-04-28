#include<stdio.h>
void print_bin(int num)
{
int pos;
for(pos=15;pos>=0;pos--)
printf("%d",num>>pos&1);
printf("\n");
}

void main()
{
unsigned short int num;
int count;
printf("Enter the number:\n");
scanf("%hd",&num);
printf("Before: %d\n",num);
print_bin(num);
num=num|(1<<15);
printf("After: %d\n",num);
print_bin(num);
printf("%d",num);
}
