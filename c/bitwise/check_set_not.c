#include<stdio.h>
void main()
{
int num,pos;
printf("Enter the number:\n");
scanf("%d",&num);
printf("Enter the pos:\n");
scanf("%d",&pos);
if(num&(1<<pos))
printf("Bit is set\n");
else
printf("Bit is not set\n");
}
