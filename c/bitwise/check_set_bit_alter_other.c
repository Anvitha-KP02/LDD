#include<stdio.h>
void print_binary(int num)
{
	int pos;
	for(pos=31;pos>=0;pos--)
	{
		printf("%d",num>>pos&1);
	}
	printf("\n");
}

void set_bit(int num,int pos)
{
	int bit1,bit2;
	if(num>>pos&1)
	{
		num &= (1<<12)|(1<<13);
		bit1=(num>>27)&1;
		bit2=(num>>28)&1;
		num |= (bit1<<12)|(bit2<<13);
	}
	print_binary(num);
}	
void main()
{
	unsigned int num;
	int pos;
	printf("Enter the number:\n");
	scanf("%u",&num);
	printf("Enter the pos:\n");
	scanf("%d",&pos);
	print_binary(num);
	set_bit(num,pos);
}
