/// Optimized way

#include<stdio.h>
#include<string.h>
void main()
{
int num,n,i;
printf("Enter the range:\n");
scanf("%d",&n);
int a[n];
memset(a,0,sizeof(a));
for(num=0;num<=n;num++)
{
a[num]=a[num>>1]+(num&1);
}
for(i=0;i<=n;i++)
printf("%d ",a[i]);
printf("\n");
}


/*#include<stdio.h>
void main()
{
int num,n,pos,set,a[n],i;
printf("Enter the number range to find the set bit count:\n");
scanf("%d",&n);
for(num=0,i=0;num<=n;num++,i++)
{
for(pos=31,set=0;pos>=0;pos--)
{
if(num>>pos&1)
{
set++;
printf("num %d having %d number of set bits\n",num,set);
a[i]=set;
}
}
}
for(i=0;i<=n;i++)
printf("%d ",a[i]);
printf("\n");
}*/
