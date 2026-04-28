#include<stdio.h>
void main()
{
	int num,set=0,pos;
	printf("Enter the number:\n");
	scanf("%d",&num);
	for(pos=31;pos>=0;pos--)
	{
	if(num>>pos&1)
		set++;
	}
	printf("count of set bits=%d\n",set);
}
