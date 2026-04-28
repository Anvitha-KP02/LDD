#include<stdio.h>
void main()
{
char s[50];
int i,j,c;
printf("Enter the string:\n");
scanf("%s",s);
for(i=0,c=1;s[i];i++)
{
if(s[i]==s[i+1])
c++;
else
{
printf("%c%d",s[i],c);
c=1;
}
}
printf("\n");
}
