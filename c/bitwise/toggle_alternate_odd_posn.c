#include<stdio.h>
void print_bin(int num)
{
int pos;
for(pos=31;pos>=0;pos--)
printf("%d",num>>pos&1);
printf("\n");
}

void main()
{
unsigned int num;
int mask,result;
printf("Enter the number:\n");
scanf("%d",&num);
printf("Before number=%d\n",num);
print_bin(num);
mask=num&0xAAAAAAAA;
result=num^mask;
printf("After number=%d\n",result);
print_bin(result);
}
