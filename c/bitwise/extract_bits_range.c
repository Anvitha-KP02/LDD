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

int extract_bits(int num,int pos1,int pos2)
{
	int mask;
	if(pos1==0 && pos2==31)
		mask=0xffffffff;
	else
		mask=(((1u<<(pos2-pos1+1))-1)<<pos1);

	return (mask&num)>>pos1;
}

void main()
{
	unsigned int num,res;
	int pos1,pos2;
	printf("Enter the number:\n");
	scanf("%d",&num);
	printf("Enter the pos1 and pos2:\n");
	scanf("%d%d",&pos1,&pos2);
	print_binary(num);
	res=extract_bits(num,pos1,pos2);
	print_binary(res);
}
