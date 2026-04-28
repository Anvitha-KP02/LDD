#include<stdio.h>
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

	if(num&1)
		printf("Odd\n");
	else
		printf("even\n");
}
