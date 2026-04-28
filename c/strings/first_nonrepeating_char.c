#include<stdio.h>
void main()
{
char s[50]="aabbcd";
int i,j,k,freq[256]={0},found=0;
for(i=0;s[i];i++)
freq[s[i]]++;

for(i=0;s[i];i++)
{
if(freq[s[i]]==1)
{
printf("%c\n",s[i]);
found=1;
return;
}
}
if(found==0)
printf("No non repeating character found\n");
}


