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

int toggle_bits(int num,int i,int j)
{
	num^=(((1<<(j-i+1))-1)<<i);
	return num;
}
void main()
{
	unsigned int num;
	int i,j;
	printf("Enter the number i and j:\n");
	scanf("%d%d%d",&num,&i,&j);
	print_binary(num);
	num=toggle_bits(num,i,j);
	print_binary(num);
}
