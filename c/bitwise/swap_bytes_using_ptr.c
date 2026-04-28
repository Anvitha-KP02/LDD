#include<stdio.h>
void main()
{
	unsigned int num=0x11223344;
	char *p,*q;
int i;
printf("Before swap: %x\n",num);
for(i=0,p=(char*)&num,q=p+3;i<2;i++,p++,q--)
{
char t=*p;
*p=*q;
*q=t;
}

printf("After swap: %x\n",num);
}
