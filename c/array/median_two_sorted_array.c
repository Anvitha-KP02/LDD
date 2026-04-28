/*Problem Statement
------------------------------------------------------------------------------------
Given two sorted arrays nums1 and nums2 of size m and n respectively, return the median of the two sorted arrays.

The overall run time complexity should be O(log (m+n)).

Example 1:

Input: nums1 = [1,3], nums2 = [2]
Output: 2.00000


Example 2:
Input: nums1 = [1,2], nums2 = [3,4]
Output: 2.50000


-----------------------------------------------------------------------------------
*/

#include<stdio.h>
void main()
{
	int a[10],b[10],c[10],i,j,mid,ele1,ele2,len;
	printf("enter the number of elements of array a:\n");
	scanf("%d",&ele1);
	printf("enter the number of elements of array b:\n");
	scanf("%d",&ele2);
	printf("Enter the array a elements:\n");
	for(i=0;i<ele1;i++)
	scanf("%d",&a[i]);
	printf("Enter the array b elements:\n");
	for(i=0;i<ele2;i++)
	scanf("%d",&b[i]);
	len=ele1+ele2;
	for(i=0;i<ele1;i++)
	{
		c[i]=a[i];
	}
	for(i=0;i<ele2;i++)
	{
		c[ele1+i]=b[i];
	}

	for(i=0;i<len-1;i++)
	{
		for(j=0;j<len-1-i;j++)
		{
			if(c[j]>c[j+1])
			{
				int t=c[j];
				c[j]=c[j+1];
				c[j+1]=t;
			}
		}
	}

	/*for(i=0;i<len;i++)
		printf("%d ",c[i]);
	printf("\n");*/

	mid=len/2;

	if(len%2==0)
		printf("%d\n",(c[mid]+c[mid-1])/2);
	else
		printf("%d\n",c[mid]);

}
