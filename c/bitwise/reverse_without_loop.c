// Reverse by swapping

#include<stdio.h>
#include<string.h>
void reverse(char *start, char *end)
{
if(start>=end)
return;

char temp=*start;
*start=*end;
*end=temp;

reverse(start+1,end-1);
}

void main()
{
char s[50];
printf("Enter the string:\n");
scanf("%s",s);
reverse(s,s+strlen(s)-1);
printf("Reversed string is %s\n",s);
}
