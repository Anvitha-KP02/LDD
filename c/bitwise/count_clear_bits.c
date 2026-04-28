#include<stdio.h>
void main()
{
	int num,set=0,clr=0,pos;
	printf("Enter the number:\n");
	scanf("%d",&num);
	for(pos=31;pos>=0;pos--)
	{
		printf("%d",num>>pos&1);
		if(pos%8==0)
			printf(" ");
	}
	printf("\n");

	for(pos=31;pos>=0;pos--)
	{
		if(num>>pos&1)
			set++;
		else
			clr++;
	}
	printf("set bits=%d  clear/reset bits=%d\n",set,clr);
}
