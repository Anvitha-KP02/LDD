#include<stdio.h>
void main()
{
char s[50];
int i,j,len;
printf("Enter the string\n");
scanf("%s",s);
len=strlen(s);
for(i=0,j=len-1;i<j;i++,j--)
{
if(s[i]=='a' || s[i]=='e' || s[i]=='i' || s[i]=='o' || s[i]=='u' || s[i]=='A' || s[i]=='E' || s[i]=='I' || s[i]=='O' || s[i]=='U')
{
char t
