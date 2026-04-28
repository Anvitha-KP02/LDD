#include<stdio.h>
void main()
{
int num,c=0;
printf("Enter the number:\n");
scanf("%d",&num);
while(num)
{
num=num&(num-1);
c++;
}
printf("count=%d\n",c);
}
