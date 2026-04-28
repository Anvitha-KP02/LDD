#include<stdio.h>
#include<string.h>
void main()
{
char s[100],temp;
int i,len,j;
printf("Enter the string:\n");
scanf("%s",s);
len=strlen(s);
for(i=0,j=len-1;i<j;i++,j--)
{
temp=s[i];
s[i]=s[j];
s[j]=temp;
}
printf("%s",s);
}


/*#include<stdio.h>
void main()
{
char s[100];
char *p,*q;
printf("Enter the string:\n");
scanf("%s",s);
p=s;
q=p;
while(*q)
q++;
q--;

while(p<q)
{
char temp=*p;
*p=*q;
*q=temp;
p++;
q--;
}
printf("%s\n",s);
}*/
