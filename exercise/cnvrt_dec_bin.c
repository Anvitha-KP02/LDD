// decimal to binary conversion

/*#include<stdio.h>
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
}*/

// decimal to binary using function

/*#include<stdio.h>
void dec_to_bin(int num);
void main()
{
int num,pos;
printf("Enter the number:\n");
scanf("%d",&num);
dec_to_bin(num);
}
void dec_to_bin(int num)
{
int pos;
for(pos=31;pos>=0;pos--)
{
printf("%d",num>>pos&1);
}
printf("\n");
}*/

// decimal to binary using recursion

#include<stdio.h>
void dec_to_bin(int num);
void main()
{
int num,pos;
printf("Enter the number:\n");
scanf("%d",&num);
dec_to_bin(num);
}
void dec_to_bin(int num)
{
static int pos=31;
printf("%d",num>>pos&1);
pos--;
if(pos>=0)
dec_to_bin(num);
}
