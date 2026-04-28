#include<stdio.h>
void main()
{
int a[]={19,20,52,67,35},ele,i,key=92,found=0;
ele=sizeof(a)/sizeof(a[0]);
for(i=0;i<ele;i++)
{
if(a[i]==key)
{
printf("key found at index %d\n",i);
found=1;
break;
}
}
if(found==0)
{
printf("Not found\n");
}
}
