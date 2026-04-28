/*#include<stdio.h>
void main()
{
char a[10]="anvitha",i,j;
int freq[10]={0};
for(i=0;a[i];i++)
freq[a[i]]++;

for(i=0;i<10;i++)
{
if(freq[i]!=0)
printf("%c: ",i);
for(j=0;j<freq[i];j++)
printf("*");
printf("\n");
}
}
*/


#include<stdio.h>
void main()
{
char a[10]="anvitha";
int freq[256]={0},i;
for(i=0;a[i];i++)
freq[a[i]]++;

for(i=0;i<256;i++)
{
if(freq[i]!=0)
printf("%c ----> %d times\n",i,freq[i]);
}
}

