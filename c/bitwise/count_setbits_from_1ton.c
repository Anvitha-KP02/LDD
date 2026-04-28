/*#include<stdio.h>
void main()
{
int num,n,c;
printf("Enter till where u want the set bit count:\n");
scanf("%d",&n);
for(num=0,c=0;num<n;num++)
{
int temp=num;
while(temp)
{
temp=temp&(temp-1);
c++;
}
printf("Set bit count for num %d is %d\n",num,c);
c=0;
}
}
*/

#include <stdio.h>
int countSetBits(int n)
{
int i=0,count=0;
while((1<<i)<=n)
{
int blockSize=1<<(i+1);
int fullBlock=(n+1)/blockSize;
count+=fullBlock*(blockSize/2);

int remainder=(n+1)%blockSize;
if(remainder>blockSize/2)
{
count+=(remainder-blockSize/2);
}
i++;
}
return count;
}

void main()
{
int num;
printf("Enter the number:\n");
scanf("%d",&num);
int result=countSetBits(num);
printf("Count of set bits from 1 to %d is %d\n",num,result);
}
