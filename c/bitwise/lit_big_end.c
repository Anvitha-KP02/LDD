/*#include<stdio.h>
void main()
{
unsigned int num=0x12345678;
unsigned char *p=(char*)&num;
if(*p==(num & 0xFF))
printf("Little endian\n");
else
printf("Big endian\n");
}*/

#include<stdio.h>
void main()
{
unsigned int num=0xffffffff;
unsigned char *p=(char*)&num;
if(*p==(0xff))
printf("Little endian\n");
else
printf("Big endian\n");
}
