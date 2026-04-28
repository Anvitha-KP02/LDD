#include<stdio.h>
void main()
{
	unsigned short int num,r1,r2;
	printf("Enter the number:\n");
	scanf("%hx",&num);
	r1=num&0x00ff;
	r1=r1<<8;
	r2=num&0xff00;
	r2=r2>>8;
	num=r1|r2;
	printf("%hx\n",num);
}

