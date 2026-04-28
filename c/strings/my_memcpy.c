#include<stdio.h>
void *my_memcpy(void* dest,const void* src,size_t n)
{
	char *d=(char*)dest;
	const char* s=(const char*)src;
	while(n--)
	{
		*d++=*s++;
	}
	return dest;
}

void main()
{
	char s[50],d[50];
	size_t n;
	printf("Enter source string:\n");
	scanf("%s",s);
	printf("Enter number of bytes to copy:\n");
	scanf("%ld",&n);
	my_memcpy(d,s,n);
	printf("Copied string: %s\n",d);
}
