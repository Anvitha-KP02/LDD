#include<stdio.h>
void main()
{
char s[50],ch;
int num=0,digit,i,j;
printf("Enter the string:\n");
scanf("%s",s);
for(i=0;s[i];i++)
{
if(s[i]>='a' && s[i]<='z')
{
if(num>0)
{
for(j=0;j<num;j++)
printf("%c",ch);
}
ch=s[i];
num=0;
}
else if(s[i]>='0' && s[i]<='9')
{
num=num*10+(s[i]-'0');
}
}
if(num>0)
{
for(j=0;j<num;j++)
printf("%c",ch);
}
printf("\n");
}

