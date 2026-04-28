#include<stdio.h>
void main()
{
int a[20],ele,i,res=0;
printf("Enter the number of elements:\n");
scanf("%d",&ele);
printf("Enter the array elements:\n");
for(i=0;i<ele;i++)
scanf("%d",&a[i]);
for(i=0;i<ele;i++)
res=res^a[i];
printf("%d\n",res);
}
