#include<stdio.h>
void print_binary(int num)
{
int pos;
for(pos=31;pos>=0;pos--)
printf("%d",num>>pos&1);
printf("\n");
}

int swap_nibble(int num)
{
return ((num&0xf0f0f0f0)>>4)|((num&0x0f0f0f0f)<<4);
}
void main()
{
unsigned int num;
printf("Enter the number:\n");
scanf("%x",&num);
printf("Before: num=%x\n",num);
print_binary(num);
num=swap_nibble(num);
printf("After: num=%x\n",num);
print_binary(num);
}
