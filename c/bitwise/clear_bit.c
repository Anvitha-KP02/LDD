#include<stdio.h>
void print_binary(int num)
{
	int pos;
	for(pos=31;pos>=0;pos--)
	{
		printf("%d",num>>pos&1);
		if(pos%8==0)
			printf(" ");
	}
	printf("\n");
}

void main()
{
	int num,pos;
	printf("Enter the number:\n");
	scanf("%d",&num);
	printf("Enter the pos to clear:\n");
	scanf("%d",&pos);
	printf("Before: num=%d\n",num);
	print_binary(num);
	num=num&~(1<<pos);
	printf("After: num=%d\n",num);
	print_binary(num);
}
