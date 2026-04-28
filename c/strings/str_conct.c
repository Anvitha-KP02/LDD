#include <stdio.h>

char *my_strcat(char *dest, const char *src)
{
 //   char *temp = dest;

    // move to end of dest
    while(*dest)
        dest++;

    // copy src to dest
    while((*dest++ = *src++))
        ;

    return dest;
}

int main()
{
    char dest[50] = "Hello ";
    char src[] = "World";

    my_strcat(dest, src);

    printf("Result: %s\n", dest);

    return 0;
}

/*#include<stdio.h>
void main()
{
char s[100],d[100];
int len,i,j;
printf("Enter the source string:\n");
scanf("%s",s);
printf("Enter the destinetion string:\n");
scanf("%s",d);
for(len=0;s[len];len++);
s[len]=' ';
for(i=0,j=len+1;d[i];i++,j++)
s[j]=d[i];
s[j]=d[i];

printf("%s\n",s);
}*/
