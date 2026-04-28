#include<stdio.h>
void main()
{
	int a[10],ele,i;
	int *p,*q;
	printf("Enter number of elements:\n");
	scanf("%d",&ele);
	printf("Enter array elements:\n");
	for(i=0;i<ele;i++)
		scanf("%d",&a[i]);
	p=a;
	q=p+(ele-1);
	while(p<q)
	{
		int temp=*p;
		*p=*q;
		*q=temp;
		p++;
		q--;
	}
	for(i=0;i<ele;i++)
		printf("%d ",a[i]);
	printf("\n");
}
