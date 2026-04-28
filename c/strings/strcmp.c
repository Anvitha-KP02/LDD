#include <stdio.h>

int my_strcmp(const char *s1, const char *s2)
{
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

int main()
{
    char s1[] = "app";
    char s2[] = "app";

    int res = my_strcmp(s1, s2);

    if(res == 0)
        printf("Strings are equal\n");
    else if(res < 0)
        printf("s1 is smaller\n");
    else
        printf("s1 is greater\n");

    return 0;
}

/*#include<stdio.h>
void main()
{
char s1[50],s2[50];
int i;
printf("Enter s1 and s2:\n");
scanf("%s%s",s1,s2);
for(i=0;s1[i]&&s2[i];i++)
{
if(s1[i]!=s2[i])
break;
}
if(s1[i]==s2[i])
printf("strings are equal\n");
else
printf("strings are not equal\n");
}*/
