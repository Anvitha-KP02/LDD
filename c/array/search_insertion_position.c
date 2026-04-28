/*Problem Statement
------------------------------------------------------------------------------------
Given a sorted array of distinct integers and a target value, return the index if the target is found. If not, return the index where it would be if it were inserted in order.

You must write an algorithm with O(log n) runtime complexity.

Example 1:
Input: nums = [1,3,5,6], target = 5
Output: 2

Example 2:
Input: nums = [1,3,5,6], target = 2
Output: 1

Example 3:
Input: nums = [1,3,5,6], target = 7
Output: 4
-----------------------------------------------------------------------------------
*/

#include<stdio.h>
void main()
{
	int a[10],ele,i,target,flag=0;;
	printf("Enter the number elements:\n");
	scanf("%d",&ele);
	printf("Enter the array elements:\n");
	for(i=0;i<ele;i++)
		scanf("%d",&a[i]);
	printf("Enter the target value:\n");
	scanf("%d",&target);

	for(i=0;i<ele;i++)
	{
		if(a[i]==target)
		{
			printf("The target is present at the index %d\n",i);
			flag=1;
			break;
		}
		if(a[i]>target)
		{
			printf("The target can inserted at the index %d\n",i);
			flag=1;
			break;
		}
	}
	if(flag==0)
			printf("The target can inserted at the index %d\n",ele);

}
