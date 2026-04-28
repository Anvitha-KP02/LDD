#include<stdio.h>
void add(int x, int y)
{
	printf("Addition: %d\n",x+y);
}
void sub(int x, int y)
{
	printf("Subtraction: %d\n",x-y);
}
void mul(int x, int y)
{
	printf("Multiplication: %d\n",x*y);
}
void div(int x, int y)
{
	printf("Division: %d\n",x/y);
}
void callback(int x,int y,void(*op)(int,int))
{
	op(x,y);
}

void main()
{
	callback(20,10,add);
	callback(20,10,sub);
	callback(20,10,mul);
	callback(20,10,div);
}
