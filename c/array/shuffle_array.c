/* Problem Statement - Shuffle array
----------------------------------------------------------------------------------------------------------
Given the array nums consisting of 2n elements in the form [x1,x2,...,xn,y1,y2,...,yn].
Return the array in the form [x1,y1,x2,y2,...,xn,yn].

Example 1:

Input: nums = [2,5,1,3,4,7], n = 3
Output: [2,3,5,4,1,7] 
Explanation: Since x1=2, x2=5, x3=1, y1=3, y2=4, y3=7 then the answer is [2,3,5,4,1,7].

----------------------------------------------------------------------------------------------------------
*/

#include<stdio.h>
void main()
{
	int a[10],ele,b[10],n,i;
	printf("Enter the even number of elements:\n");
	scanf("%d",&ele);
	printf("Enter the array elements:\n");
	for(i=0;i<ele;i++)
		scanf("%d",&a[i]);
	printf("Enter the number of elements to divide:\n");
	scanf("%d",&n);

	for(i=0;i<ele;i++)
	{
		b[2*i]=a[i];
		b[2*i+1]=a[n+i];
	}

	for(i=0;i<ele;i++)
		printf("%d ",b[i]);
	printf("\n");
}
