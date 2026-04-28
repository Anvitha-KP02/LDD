#include<stdio.h>
void print_bin(int num)
{
int pos;
for(pos=31;pos>=0;pos--)
printf("%d",num>>pos&1);
printf("\n");
}
void main()
{
int num,i;
printf("Enter the num:\n");
scanf("%d",&num);
print_bin(num);
num=(num&0xAAAAAAAA)>>1|(num&0x55555555)<<1;
num=(num&0xcccccccc)>>2|(num&0x33333333)<<2;
num=(num&0xf0f0f0f0)>>4|(num&0x0f0f0f0f)<<4;
num=(num&0xff00ff00)>>8|(num&0x00ff00ff)<<8;
num=(num>>16)|(num<<16);
print_bin(num);
}
