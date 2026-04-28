//  Find the single number in an array where all others appear twice
//  Array = [2, 3, 5, 4, 5, 3, 4]
//  Answer = 2


#include<stdio.h>
void main()
{
	int a[11]={2,3,5,4,5,3,4,2,6,6,1},i,res=0;
	for(i=0;i<11;i++)
	{
		res=res^a[i];
	}
	printf("Result=%d\n",res);
}
