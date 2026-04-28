#include<stdio.h>
int avg_arr(int *a,int ele)
{
	int i,sum=0,avg;
	for(i=0;i<ele;i++)
	{
		sum=sum+a[i];
	}
	avg=sum/ele;
	return avg;
}

void main()
{
	int (*func_ptr)(int[],int)=avg_arr;
	int a[5]={1,2,3,4,5,6,7,8,9,2},ele;
	ele=sizeof(a)/sizeof(a[0]);
	int avg;
	avg=(*func_ptr)(a,ele);
	printf("Average of array elements: %d\n",avg);
}


/*#include<stdio.h>
void main()
{
	int a[5]={1,2,3,4,5},sum=0,avg,i;
	for(i=0;i<5;i++)
	{
		sum=sum+a[i];
	}
	avg=sum/5;

	printf("Average of array elements: %d\n",avg);
}*/
