#include<stdio.h>
void add(int x,int y)
{
	printf("Addition:%d\n",x+y);
}
void sub(int x,int y)
{
	printf("Subtraction:%d\n",x-y);
}

void main()
{
	char ch;
	void (*ptr)(int,int);
	printf("Enter the choice:\n");
	scanf(" %c",&ch);
	if(ch=='+')
		ptr=add;
	else if(ch=='-')
		ptr=sub;
	else
	{
		printf("Enter the valid choice:\n");
		return;
	}
	ptr(20,10);
}
