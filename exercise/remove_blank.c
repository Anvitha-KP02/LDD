#include<stdio.h>
void main()
{
char s[50];
int c,i,j;
printf("Enter the string:\n");
scanf("%[^\n]",s);
for(i=0;s[i];i++)
{
if(s[i]==' ' && s[i+1]==' ')
{
for(j=i;s[j];j++)
{
s[j]=s[j+1];
}
i--;
}
}
printf("%s\n",s);
}

/*#include<stdio.h>
void main()
{
char s[50];
int c=0,i,j;
printf("Enter the string with the blanks:\n");
scanf("%[^\n]",s);
for(i=0;s[i];i++)
{
if(s[i]==' ')
c++;
if(c>1)
{
for(j=i;s[j];j++)
{
s[j]=s[j+1];
c=0;
}
i--;
}
}
printf("%s\n",s);
}*/
