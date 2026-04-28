#include<stdio.h>
void main()
{
int a[]={2,5,8,12,35,78},ele,l,r,mid,key=12;
ele=sizeof(a)/sizeof(a[0]);
l=0;
r=ele-1;
while(l<=r)
{
mid=(l+r)/2;
if(key==a[mid])
{
printf("key found at index %d\n",mid);
return;
}	
else if(key < a[mid])
r=mid-1;
else
l=mid+1;
}
printf("Key not found\n");
}
