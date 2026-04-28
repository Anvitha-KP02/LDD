// Swap even and odd bits of the number. Swaps every pair of bits in a 32-bit integer

//Bit 0 ↔ Bit 1

//Bit 2 ↔ Bit 3

//Bit 4 ↔ Bit 5

//… and so on

#include<stdio.h>
void main()
{
	int num,pos;
	printf("Enter the number:\n");
	scanf("%d",&num);
	for(pos=31;pos>=0;pos--)
	{
		printf("%d",num>>pos&1);
	}
	printf("\n");
	int num1,num2;
	num1=num & 0xAAAAAAAA;  //to get odd bits
	num2=num & 0x55555555;  // to get even bits
	num=(num1>>1)|(num2<<1);

	printf("%d\n",num);

	for(pos=31;pos>=0;pos--)
	{
		printf("%d",num>>pos&1);
	}
	printf("\n");
}
