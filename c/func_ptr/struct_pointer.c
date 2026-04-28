#include<stdio.h>
#include<stdlib.h>
struct data
{
int a;
int b;
int res;
};

void main()
{
struct data *d=malloc(sizeof(struct data));
printf("Enter a and b:\n");
scanf("%d%d",&d->a,&d->b);
d->res=d->a+d->b;
printf("Result=%d\n",d->res);
}
