#include<stdio.h>
void main()
{
int a[]={1,2,3,4};
int ele,i,right;
ele=sizeof(a)/sizeof(a[0]);
int b[ele];
b[0]=1;
for(i=1;i<ele;i++)
{
b[i]=b[i-1]*a[i-1];
}

right=1;
for(i=ele-1;i>=0;i--)
{
b[i]=b[i]*right;
right=right*a[i];
}

for(i=0;i<ele;i++)
printf("%d ",b[i]);
printf("\n");
}
