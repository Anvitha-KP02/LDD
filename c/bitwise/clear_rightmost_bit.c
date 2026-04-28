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

void main()
{
	int num;
	printf("Enter the number:\n");
	scanf("%d",&num);
	print_binary(num);
	num=num&(num-1);
	print_binary(num);
}
