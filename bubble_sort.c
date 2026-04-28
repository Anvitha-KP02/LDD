#include<stdio.h>
void main()
{
int a[]={6,3,8,12,45,1,90},ele,i,j,temp,flag=0;
ele=sizeof(a)/sizeof(a[0]);
for(i=0;i<ele-1;i++)
{
for(j=0;j<ele-1-i;j++)
{
if(a[j]>a[j+1])
{
temp=a[j];
a[j]=a[j+1];
a[j+1]=temp;
}
}
}
for(i=0;i<ele;i++)
printf("%d ",a[i]);
printf("\n");
}
