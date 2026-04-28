/*Input: s = "the sky is blue"
Output: "blue is   sky the  "*/

#include<stdio.h>
#include<string.h>
void main()
{
char s[100];
int i,j,len,start,end;
printf("Enter the string:\n");
scanf("%[^\n]",s);
len=strlen(s);
i=len-1;
while(i>=0)
{
while(i>=0 && s[i]==' ')
i--;

if(i<0)
break;

end=i;

while(i>=0 && s[i]!=' ')
i--;
start=i+1;

while(start<=end)
printf("%c",s[start++]);

if(i>=0)
printf(" ");
}
}
