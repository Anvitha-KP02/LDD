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
	printf("Enter the position to toggle:\n");
	scanf("%d",&pos);
	printf("num=%d\n",num);
	print_binary(num);
	num=num^(1<<pos);
	print_binary(num);
	printf("num=%d\n",num);
}	
