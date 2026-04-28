#include<stdio.h>
void main()
{
char s[50]="hello world";
int i,c,j;
for(i=0;s[i];i=i+1)
{
for(j=i,c=0;s[j]!=' ' && s[j]!='\0';j++)
c++;
printf("word length is %d\n",c);
}
}
