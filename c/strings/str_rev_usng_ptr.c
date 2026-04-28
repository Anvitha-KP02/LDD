#include<stdio.h>
void main()
{
	char s[100];
	char *p,*q;
	printf("Enter the string:\n");
	scanf("%[^\n]",s);
	p=s;
	while(*p!='\0')
	{
		q=p;
		while(*q!=' ' && *q!='\0')
			q++;
		char *start,*end;
		start=p;
		end=q-1;
		while(start<end)
		{
			char temp=*start;
			*start=*end;
			*end=temp;
			start++;
			end--;
		}
		if(*q=='\0')
			break;

		p=q+1;
	}
	printf("%s\n",s);
}

