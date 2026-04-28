#include<stdio.h>
void main()
{
unsigned int num;
printf("Enter the number:\n");
scanf("%x",&num);
printf("Before: num=%x\n",num);
num=(num&0xffff0000)>>16 | (num&0x0000ffff)<<16;
printf("%x\n",num);
}
