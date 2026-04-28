#include<stdio.h>
void main()
{
int num,n;
printf("Enter the number:\n");
scanf("%d",&num);
printf("Enter the power:\n");
scanf("%d",&n);
int res;
res=num>>n;
printf("%d\n",res);
}
