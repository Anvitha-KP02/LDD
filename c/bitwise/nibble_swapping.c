#include<stdio.h>
void main()
{
unsigned int num;
printf("Enter the number:\n");
scanf("%x",&num);
printf("Before Num=%x\n",num);
num=(num&0xf0f0f0f0)>>4 | (num&0x0f0f0f0f)<<4 ;
printf("After num=%x\n",num);
}
