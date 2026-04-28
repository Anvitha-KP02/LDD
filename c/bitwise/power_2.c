#include<stdio.h>
void main()
{
	int num;
	printf("Enter the number:\n");
	scanf("%d",&num);
	if(num&(num-1))
		printf("Odd\n");
	else
		printf("Even\n");
}
