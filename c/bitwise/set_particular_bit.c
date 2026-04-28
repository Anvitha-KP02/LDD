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
	int num,pos;
	printf("Enter the number:\n");
	scanf("%d",&num);
	printf("Enter the position:\n");
	scanf("%d",&pos);
	print_binary(num);
	num=num|(1<<pos);
	print_binary(num);
}	
