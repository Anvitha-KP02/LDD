#include<stdio.h>
void main()
{
int x,y,c=0;
printf("Enter x and y:\n");
scanf("%d%d",&x,&y);
int diff;
diff=x^y;
while(diff)
{
diff=diff&(diff-1);
c++;
}
printf("count of set bits: %d\n",c);
}
