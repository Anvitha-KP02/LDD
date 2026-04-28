//Check whether two numbers are same without using == operator.

#include<stdio.h>
void main()
{
	int a,b;
	printf("Enter a and b:\n");
	scanf("%d%d",&a,&b);

	if(a^b)
		printf("They are different\n");
	else
		printf("They are same\n");
}
