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
	int num,bit1,bit2;
	printf("Enter the num,bit1 and bit2:\n");
	scanf("%d%d%d",&num,&bit1,&bit2);
	int b1,b2;
	print_binary(num);
	b1=(num>>bit1)&1;
	b2=(num>>bit2)&1;
	if(b1!=b2)
	{
		num=num^(1<<bit1)|(1<<bit2);
	}
	printf("num=%d\n",num);
	print_binary(num);
}
