#include<stdio.h>
struct data
{
int num1;
int num2;
int res;
};
struct data d;
int add(int num1,int num2)
{
return num1+num2;
}
void main()
{
printf("Enter num1 and num2:\n");
scanf("%d%d",&d.num1,&d.num2);
d.res=add(d.num1,d.num2);
printf("Result=%d\n",d.res);
}

