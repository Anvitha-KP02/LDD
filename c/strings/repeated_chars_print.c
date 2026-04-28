#include <stdio.h>

int main()
{
    int a[10], ele, i;
    int f[100] = {0};  // assume values 0–99

    printf("Enter number of elements:\n");
    scanf("%d", &ele);

    printf("Enter array elements:\n");
    for(i = 0; i < ele; i++)
        scanf("%d", &a[i]);

    printf("Repeated elements: ");

    for(i = 0; i < ele; i++)
    {
        f[a[i]]++;

        if(f[a[i]] == 2)
        {
            printf("%d ", a[i]);
        }
    }

    printf("\n");

    return 0;
}

/*#include<stdio.h>
void main()
{
char s[100];
int i,j,k;
printf("Enter the string:\n");
scanf("%[^\n]",s);
for(i=0;s[i];i++)
{
for(j=i+1;s[j];j++)
{
if(s[i]==s[j])
{
printf("Repeated character:%c\n",s[j]);
for(k=j;s[k];k++)
{
s[k]=s[k+1];
}
j--;
}
}
}
printf("%s\n",s);
}*/
