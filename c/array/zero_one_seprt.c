// Input: a[10]={1,0,1,1,0,0,0,1,1,0};
// Output: a[10]={0,0,0,0,0,1,1,1,1,1};

#include<stdio.h>
void main()
{
//int a[10]={1,0,1,1,0,0,0,1,1,0};
//int ele=sizeof(a)/sizeof(a[0]);
int a[20],ele;
int j,t,i;
printf("Enter the number of elements:\n");
scanf("%d",&ele);
printf("Enter the array elements:\n");
for(i=0;i<ele;i++)
scanf("%d",&a[i]);
for(i=0,j=ele-1;i<j;i++)
{
if(a[i]!=a[j])
{
if(a[i]==0)
{
int t=a[i];
a[i]=a[j];
a[j]=t;
}
j--;
}
}
for(j=0;j<ele;j++)
printf("%d ",a[j]);
printf("\n");
}

/*#include<stdio.h>
void main()
{
int a[20],ele;
int j,t;
printf("Enter the number of elements:\n");
scanf("%d",&ele);
printf("Enter the array elements:\n");
for(int i=0;i<ele;i++)
scanf("%d",&a[i]);
int i=0;
for(j=0;j<ele;j++)
{
if(a[j]==0)
{
t=a[i];
a[i]=a[j];
a[j]=t;
i++;
}
}
for(j=0;j<ele;j++)
printf("%d ",a[j]);
printf("\n");
}
*/

/*#include<stdio.h>
void main()
{
int a[10]={1,0,1,1,0,0,0,1,1,0};
int i=0,j,t;
for(j=0;j<10;j++)
{
if(a[j]==0)
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
}*/
