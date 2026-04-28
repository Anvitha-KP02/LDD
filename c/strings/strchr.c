//my_strchr

#include <stdio.h>

char *my_strchr(const char *s, char ch)
{
    while(*s)
    {
        if(*s == ch)
            return (char *)s;

        s++;
    }

    // check for '\0' case
    if(ch == '\0')
        return (char *)s;

    return NULL;
}

int main()
{
    char str[] = "embedded";
    char ch = 'd';

    char *res = my_strchr(str, ch);

    if(res)
        printf("Character found at address: %p, position: %ld\n", (void*)res, res - str);
    else
        printf("Character not found\n");

    return 0;
}

/*#include<stdio.h>
void main()
{
char s[100],ch;
int i,len;
printf("Enter the string:\n");
scanf("%s",s);
printf("Enter the character:\n");
scanf(" %c",&ch);
for(i=0;s[i];i++)
{
if(s[i]==ch)
{
printf("Char is present\n");
return;
}
}
printf("char is not present\n");
}*/
