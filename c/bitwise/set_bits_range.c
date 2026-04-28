#include<stdio.h>
void print_binary(int num)
{
int pos;
for(pos=31;pos>=0;pos--)
printf("%d",num>>pos&1);
printf("\n");
}

int setbits_in_range(int num,int i,int j)
{
num|=(((1<<(j-i+1))-1)<<i);
return num;
}

void main()
{
unsigned int num;
int i,j;
printf("Enter the number and the range i and j:\n");
scanf("%d%d%d",&num,&i,&j);
printf("Before: num=%d\n",num);
print_binary(num);
num=setbits_in_range(num,i,j);
printf("After: num=%d\n",num);
print_binary(num);
}
