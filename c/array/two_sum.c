/*Problem Statement
------------------------------------------------------------------------------------
Given an array of integers nums and an integer target, return indices of the two numbers such that they add up to target.

You may assume that each input would have exactly one solution, and you may not use the same element twice.

You can return the answer in any order.

 

Example 1:
Input: nums = [2,7,11,15], target = 9
Output: [0,1]

Example 2:
Input: nums = [3,2,4], target = 6
Output: [1,2]

-----------------------------------------------------------------------------------
*/

#include<stdio.h>
void main()
{
	int a[10],ele,i,j,target;
	printf("Enter the number of elements:\n");
	scanf("%d",&ele);
	printf("Enter the array elements:\n");
	for(i=0;i<ele;i++)
		scanf("%d",&a[i]);
	printf("Enter the target:\n");
	scanf("%d",&target);

	for(i=0;i<ele;i++)
	{
		for(j=i+1;j<ele;j++)
		{
			if(target==a[i]+a[j])
			{
				printf("%d %d\n",i,j);
			}
		}
	}
}
