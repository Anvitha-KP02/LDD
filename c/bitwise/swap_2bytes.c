#include<stdio.h>
void main()
{
unsigned int num;
printf("Enter the number:\n");
scanf("%x",&num);
printf("Before:NUM=%x\n",num);
num=(num&0xff00ff00)>>8 | (num&0x00ff00ff)<<8;
printf("After: NUM=%x\n",num);
}
