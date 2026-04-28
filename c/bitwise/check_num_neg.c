#include<stdio.h>
void main()
{
int num;
printf("Enter the number\n");
scanf("%d",&num);
if((num>>31)&1==1)
printf("Negative number\n");
else
printf("Positive number\n");
}
