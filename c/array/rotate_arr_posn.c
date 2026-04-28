#include<stdio.h>
void main()
{
int a[20],ele,i,j,k,temp;
printf("Enter the number of elements:\n");
scanf("%d",&ele);
printf("Enter the array elements:\n");
for(i=0;i<ele;i++)
scanf("%d",&a[i]);
printf("Enter the number of rotation:\n");
scanf("%d",&k);
for(j=0;j<=k;j++)
{
temp=a[0];
for(i=0;i<ele;i++)
{
a[i]=a[i+1];
}
a[ele-1]=temp;
}

for(i=0;i<ele;i++)
printf("%d ",a[i]);
printf("\n");
}
