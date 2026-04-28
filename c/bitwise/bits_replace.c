#include<stdio.h>

void print_binary(int num)
{
int pos;
for(pos=31;pos>=0;pos--)
printf("%d",num>>pos&1);
printf("\n");
}

void main()
{
int num1,num2,bits,pos;
printf("Enter the number which replace the bits of other number:\n");
scanf("%d",&num1);
printf("Enter the number of bits to extract and replace:\n");
scanf("%d",&bits);
printf("Enter the number which gets altered:\n");
scanf("%d",&num2);
printf("Enter the position from where it gets replace:\n");
scanf("%d",&pos);

print_binary(num1);
print_binary(num2);

num1=num1&(num1<<(pos+1-bits)); 
num2=num2|num1;

printf("After :\n");
print_binary(num2);
}
