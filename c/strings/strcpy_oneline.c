#include<stdio.h>
void my_strcpy(const char *s,char *d);
void print_str(const char *p);
void main()
{
char s[20],d[20];
printf("Enter src string:\n");
scanf("%s",s);
print_str(d);
my_strcpy(s,d);
print_str(d);
}
void my_strcpy(const char *s,char *d)
{
while(*s)
*d++=*s++;
*d=*s;
}

void print_str(const char *p)
{
while(*p)
printf("%c",*p++);
printf("\n");
}


/*#include<stdio.h>
void main()
{
char s[50],d[50];
int i;
printf("Enter the source string:\n");
scanf("%s",s);
printf("Source string: %s\n",s);
for(i=0;d[i]=s[i];i++);
printf("The source string and dest string is: %s %s\n",s,d);
}*/
