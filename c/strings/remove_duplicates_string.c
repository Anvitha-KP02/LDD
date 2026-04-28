#include <stdio.h>

int main()
{
    char s[100];
    int freq[256] = {0};
    int i, count = 0;

    printf("Enter string:\n");
    scanf("%s", s);

    printf("Unique characters: ");

    for(i = 0; s[i]; i++)
    {
        if(freq[s[i]] == 0)
        {
            printf("%c", s[i]);
            freq[s[i]] = 1;
        }
        else
        {
            count++;
        }
    }

    printf("\nCount = %d\n", count);

    return 0;
}


/*#include<stdio.h>
void main()
{
	char s[50];
	int a[256],i,k=0;
	printf("Enter the string:\n");
	scanf("%[^\n]",s);
	memset(a,0,sizeof(a));
	for(i=0;s[i];i++)
	{
		if(s[i]==' ')
		{
			s[k++]=s[i];
			continue;
		}
		if(!a[s[i]])
		{
			a[s[i]]=1;
			s[k++]=s[i];
		}
	}
	s[k]='\0';
	printf("%s\n",s);
}*/


/*#include<stdio.h>
  void main()
  {
  char s[50];
  printf("Enter the string:\n");
  scanf("%[^\n]",s);
  int i,j,k;
  for(i=0;s[i];i++)
  {
  for(j=i+1;s[j];j++)
  {
  if(s[i]==s[j])
  {
  for(k=j;s[k];k++)
  s[k]=s[k+1];
  j--;
  }
  }
  }
  printf("%s\n",s);
  }*/
