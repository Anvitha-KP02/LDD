// Input: a[10]={-1,2,9,-3,5,4,-6,8,1,-9};
// Output: a[10]={-1,-3,-6,-9,2,9,5,4,8,1};

#include<stdio.h>
void main()
{
int a[10]={-1,2,9,-3,5,4,-6,8,1,-9};
int i=0,j,t;
for(j=0;j<10;j++)
{
if(a[j]<0)
{
t=a[i];
a[i]=a[j];
a[j]=t;
i++;
}
}
for(j=0;j<10;j++)
printf("%d ",a[j]);
printf("\n");
}
