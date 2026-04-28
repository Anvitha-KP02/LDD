// input: a[8]={1,2,1,3,2,3,4,5};
// output: 4,5

#include <stdio.h>
void main()
{
int a[]={1,2,1,3,2,3,4,5};
int ele=sizeof(a)/sizeof(a[0]);
int i,res=0;
for(i=0;i<ele;i++)
{
res=res^a[i];
}

int setbit=res&(~(res-1));  // rightmost setbit

int x=0,y=0;

for(i=0;i<ele;i++)
{
if(a[i]&setbit)
x=x^a[i];
else
y=y^a[i];
}
printf("Two unique elements are:%d %d\n",x,y);
}

