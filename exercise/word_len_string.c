#include<stdio.h>
void main()
{
char s[100];
int i,j,c;
printf("Enter the string:\n");
scanf("%[^\n]",s);
for(i=0;s[i];i=j+1)
{
for(j=i,c=0;s[j]!=' ' && s[j]!='\0';j++)
c++;
printf("%d \n",c);
}
if(s[j]=='\0')
c++;
//printf("%d\n",c);
}
