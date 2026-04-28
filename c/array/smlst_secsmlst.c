#include<stdio.h>
void main()
{
int a[]={7,3,8,2,1,9},ele,i,S,SS;
ele=sizeof(a)/sizeof(a[0]);
if(a[0]<a[1])
{
S=a[0];
SS=a[1];
}
else if(a[1]<a[0])
{
S=a[1];
SS=a[0];
}
for(i=2;i<ele;i++)
{
if(a[i]<S)
{
SS=S;
S=a[i];
}
else if(a[i]<SS &&a[i]!=S)
SS=a[i];
}
printf("Smallest=%d and sec smallest=%d\n",S,SS);
}
