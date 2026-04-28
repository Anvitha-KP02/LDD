#include<stdio.h>
void main()
{
int a[]={26,78,11,9,3,67},ele,i,j,temp;
ele=sizeof(a)/sizeof(a[0]);
for(i=0;i<ele-1;i++)
{
for(j=i+1;j<ele;j++)
{
if(a[i]>a[j])
{
temp=a[i];
a[i]=a[j];
a[j]=temp;
}
}
}
for(i=0;i<ele;i++)
printf("%d ",a[i]);
printf("\n");
}
