#include<stdio.h>
void main()
{
int ele,i;
printf("Enter array elements:\n");
scanf("%d",&ele);
int a[ele];
int *p=a;
printf("Enter array elements:\n");
for(i=0;i<ele;i++)
scanf("%d",&a[i]);
for(i=0;i<ele;i++)
printf("%d ",a[i]);
printf("\n");
}

/*#include<stdio.h>
#include<stdlib.h>
void main()
{
int ele,i;
printf("Enter the number of elements:\n");
scanf("%d",&ele);
int *p=malloc(sizeof(int));
printf("Enter array elements:\n");
for(i=0;i<ele;i++)
scanf("%d",&p[i]);
for(i=0;i<ele;i++)
printf("%d ",p[i]);
printf("\n");
}*/
