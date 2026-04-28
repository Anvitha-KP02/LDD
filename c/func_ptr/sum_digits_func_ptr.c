#include<stdio.h>

int add(int a, int b)
{
	return a+b;
}

void main()
{
	int sum;
	int (*func_ptr)(int, int)=&add;
	sum=(*func_ptr)(2,3);
	printf("Sum=%d\n",sum);
}
