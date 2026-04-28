// Find the missing element in the array where the sequence should start from 1.

/*#include<stdio.h>
void to_print(int *arr,int n)
{
	for(int i=0;i<n;i++)
		printf("%d ",arr[i]);
}
int to_find_missing(int *arr,int n)
{
	int res1=0,res2=0;
	for(int i=0;i<n;i++)
		res1^=arr[i];
	for(int i=1;i<=n+1;i++)
		res2^=i;
	return res1^res2;
}
int main()
{
	int n;
	printf("enter  a n value:");
	scanf("%d",&n);
	int arr[n];
	printf("enter array elements:");
	for(int i=0;i<n;i++)
		scanf("%d",&arr[i]);
	to_print(arr,n);
	printf("the  missing num is:%d",to_find_missing(arr,n));
}*/


#include<stdio.h>
void main()
{
	int n,i;
	printf("Enter n:\n");
	scanf("%d",&n);
	int arr[n];
	printf("Enter the array elements:\n");
	for(i=0;i<n;i++)
		scanf("%d",&arr[i]);

	int res1=0,res2=0;
	for(i=0;i<n;i++)
		res1 ^= arr[i];
	for(i=1;i<=n+1;i++)
		res2 ^= i;

	int result;
	result=res1^res2;

	printf("The missing number in the array is %d\n",result);
}
