#include<stdio.h>
void *my_memmove(void* dest, const void* src, size_t n)
{
	char *d=(char*)dest;
	const char *s=(const char*)src;

	if(d==s)
		return dest;

	if(d<s)
	{
		while(n--)
		{
			*d++=*s++;
		}
	}
	else
	{
		d=d+n-1;
		s=s+n-1;
		while(n--)
		{
			*d++=*s++;
		}
	}
	return dest;
}

void main()
{
	char s[50];
	size_t n;
	printf("Enter a string:\n");
	scanf("%s",s);
	printf("Enter number of bytes to copy:\n");
	scanf("%ld",&n);
	my_memmove(s+2,s,n);
	printf("%s\n",s);
}
