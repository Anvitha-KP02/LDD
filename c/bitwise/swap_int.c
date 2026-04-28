#include<stdio.h>
void main()
{
	int a,b,temp;
	printf("Enter a and b\n");
	scanf("%d%d",&a,&b);
	printf("before swap: a=%d b=%d\n",a,b);
/*	a=a+b;
	b=a-b;
	a=a-b;*/

/*	a=a^b;
	b=a^b;
	a=a^b;*/

	temp=a;
	a=b;
	b=temp;
	printf("After swap: a=%d b=%d\n",a,b);
}
