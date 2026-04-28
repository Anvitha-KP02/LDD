#include<stdio.h>
void main()
{
int a[]={78,45,3,76,34,90},ele,i,j,temp;
ele=sizeof(a)/sizeof(a[0]);
for(i=1;i<ele;i++)
{
temp=a[i];
j=i-1;
while(j>=0 && a[j]>temp)
{
a[j+1]=a[j];
j--;
}
a[j+1]=temp;
}
for(i=0;i<ele;i++)
printf("%d ",a[i]);
printf("\n");
}
