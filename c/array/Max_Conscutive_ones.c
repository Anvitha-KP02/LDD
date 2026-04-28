/* Problem Statement - Max Conscutive ones
----------------------------------------------------------------------------------------------------------
Given a binary array nums, return the maximum number of consecutive 1's in the array.

Example 1:

Input: nums = [1,1,0,1,1,1]
Output: 3
Explanation: The first two digits or the last three digits are consecutive 1s. The maximum number of consecutive 1s is 3.
Example 2:

Input: nums = [1,0,1,1,0,1]
Output: 2

----------------------------------------------------------------------------------------------------------
*/

#include <stdio.h>

int maxConsecutiveOnes(int *nums, int n)
{
    int count = 0, max = 0;

    for(int i = 0; i < n; i++)
    {
        if(nums[i] == 1)
        {
            count++;
            if(count > max)
                max = count;
        }
        else
        {
            count = 0;
        }
    }

    return max;
}

int main()
{
    int nums[] = {1,1,0,1,1,1};
    int n = sizeof(nums) / sizeof(nums[0]);

    int result = maxConsecutiveOnes(nums, n);

    printf("Maximum consecutive 1s = %d\n", result);

    return 0;
}

/*#include<stdio.h>
void main()
{
	int a[10],ele,i,j,c=0;
	printf("Enter the number of elements:\n");
	scanf("%d",&ele);
	printf("Enter the array elements:\n");
	for(i=0;i<ele;i++)
		scanf("%d",&a[i]);
	for(i=0;i<ele;i++)
	{
		if(a[i]==a[i+1])
			c++;
	}
	printf("Number of consecutive ones: %d\n",c);

	//for(i=0;i<ele;i++)
	//	printf("%d ",a[i]);
	//printf("\n");
}*/
