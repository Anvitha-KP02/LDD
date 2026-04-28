#include<stdio.h>
void main()
{
char s[100];
int len=0,i,j;
printf("Enter the string:\n");
scanf("%[^\n]",s);
for(len=0;s[len];len++);
for(i=0,j=len-1;i<j;i++,j--)
{
char t=s[i];
s[i]=s[j];
s[j]=t;
}

printf("%s\n",s);
}

