#include<stdio.h>

char* my_strstr(char *m, char*s)
{
	if(*s=='\0')
		return m;

	for(int i=0;m[i];i++)
	{
		if(m[i]==s[0])
		{
			int j=0;
			while(m[i+j]==s[j] && s[j]!='\0')
			{
				j++;
			}

			if(s[j]=='\0')
			{
				return &m[i];
			}
		}
	}
	return NULL;
}

void main()
{
	char s[100],m[100];
	printf("Enter the main string:\n");
	scanf("%s",m);
	printf("Enter the substring:\n");
	scanf("%s",s);

	char *res=my_strstr(m,s);
	if(res)
		printf("Substring found at address:%p\n",res);
	else
		printf("Substring not exists\n");
}
