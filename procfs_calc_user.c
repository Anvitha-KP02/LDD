#include<stdio.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include"common.h"

void main()
{
	/*int fd;
	struct data d;
	struct result r;

	printf("Enter a:\n");
	scanf("%d",&d.a);
	printf("Enter b:\n");
	scanf("%d",&d.b);
	printf("Enter op:\n");
	scanf(" %c",&d.op);
	fd=open("/proc/proc_basic",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return;
	}
	
	printf("%d %c %d\n",d.a,d.op,d.b);
	d.a = 10;
	d.b = 20;
	d.op = '+';
	write(fd,&d,sizeof(d));

	read(fd,&r,sizeof(r));
	printf("Result=%d\n",r.res);*/
}
