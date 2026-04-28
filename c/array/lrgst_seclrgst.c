#include <stdio.h>
#include <limits.h>

int main()
{
    int a[] = {7, 3, 8, 2, 1, 9};
    int ele = sizeof(a) / sizeof(a[0]);

    if(ele < 2)
    {
        printf("Not enough elements\n");
        return 0;
    }

    int L = a[0], SL = a[0];

    for(int i = 1; i < ele; i++)
    {
        if(a[i] > L)
        {
            SL = L;
            L = a[i];
        }
        else if(a[i] > SL && a[i] != L)
        {
            SL = a[i];
        }
    }

    if(SL == L)
        printf("Largest = %d and no second largest\n",L);
    else
        printf("Largest = %d, Second Largest = %d\n", L, SL);

    return 0;
}

/*#include<stdio.h>
void main()
{
int a[]={7,3,8,2,1,9},ele,i,L,SL;
ele=sizeof(a)/sizeof(a[0]);
if(a[0]>a[1])
{
L=a[0];
SL=a[1];
}
else if(a[1]>a[0])
{
L=a[1];
SL=a[0];
}
for(i=2;i<ele;i++)
{
if(a[i]>L)
{
SL=L;
L=a[i];
}
else if(a[i]>SL &&a[i]!=L)
SL=a[i];
}
printf("Largest=%d and sec largest=%d\n",L,SL);
}*/
