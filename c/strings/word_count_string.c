#include <stdio.h>

int main()
{
    char s[100];
    int i, count = 0;

    printf("Enter the string:\n");
    scanf("%[^\n]", s);

    for(i = 0; s[i]; i++)
    {
        if(s[i] != ' ' && (i == 0 || s[i-1] == ' '))
        {
            count++;
        }
    }

    printf("Count of words: %d\n", count);

    return 0;
}

/*#include<stdio.h>
void main()
{
char s[100];
int i,c=0;
printf("Enter the string:\n");
scanf("%[^\n]",s);
for(i=0;s[i];i++)
{
if(s[i]==' ')
c++;
}
c++;
printf("Count of words: %d\n",c);
}*/
