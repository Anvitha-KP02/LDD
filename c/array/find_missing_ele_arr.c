#include<stdio.h>
void main()
{
	int a[]={1,2,3,5,6},ele,i;
	ele=sizeof(a)/sizeof(a[0]);
	int xor_arr=0;
	int xor_all=0;

	for(i=0;i<ele;i++)
		xor_arr ^= a[i];
	for(i=1;i<=ele+1;i++)
		xor_all ^= i;
	int result;
	result = xor_arr ^ xor_all;
	printf("Result=%d\n",result);
}
