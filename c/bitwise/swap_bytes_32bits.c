#include<stdio.h>
void main()
{
	unsigned int num,r1,r2,r3,r4;
	printf("Enter the number:\n");
	scanf("%x",&num);
	r1=num&0x000000ff;
	r1=r1<<24;
	r2=num&0xff000000;
	r2=r2>>24;
	r3=num&0x00ff0000;
	r3=r3>>8;
	r4=num&0x0000ff00;
	r4=r4<<8;
	num=r1|r2|r3|r4;
	printf("%x\n",num);
}

