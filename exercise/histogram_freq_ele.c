#include<stdio.h>
void main()
{
int a[]={3,4,2,2,4,1,2},ele,i,j;
int freq[10]={0};
ele=sizeof(a)/sizeof(a[0]);
for(i=0;i<ele;i++)
freq[a[i]]++;

for(i=0;i<10;i++)
{
if(freq[i]!=0)
printf("%d: ",i);
for(j=0;j<freq[i];j++)
printf("*");
printf("\n");
}
}

/*#include<stdio.h>
void main()
{
int a[]={5,6,3,3,2,1,5};
int ele,i;
ele=sizeof(a)/sizeof(a[0]);
int freq[10]={0};
for(i=0;i<ele;i++)
{
freq[a[i]]++;
}

for(i=0;i<10;i++)
{
if(freq[i]!=0)
printf("%d ----> %d times\n",i,freq[i]);
}
}*/
