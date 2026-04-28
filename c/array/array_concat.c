/* Problem Statement - Concatenation of array
----------------------------------------------------------------------------------------------------------
Given an integer array nums of length n, you want to create an array ans of length 2n where ans[i] == nums[i] and ans[i + n] == nums[i] for 0 <= i < n (0-indexed).
Specifically, ans is the concatenation of two nums arrays.
Return the array ans.

Example 1:
Input: nums = [1,2,1]
Output: [1,2,1,1,2,1]
Explanation: The array ans is formed as follows:
- ans = [nums[0],nums[1],nums[2],nums[0],nums[1],nums[2]]
- ans = [1,2,1,1,2,1]
----------------------------------------------------------------------------------------------------------
*/

#include<stdio.h>
void main()
{
	int a[3]={1,2,1},ele,i,b[6];
	ele=sizeof(a)/sizeof(a[0]);
	for(i=0;i<ele;i++)
	{
		b[i]=a[i];
		b[ele+i]=a[i];
	}
	for(i=0;i<(2*ele);i++)
		printf("%d ",b[i]);
	printf("\n");
}
