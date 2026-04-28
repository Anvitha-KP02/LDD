#include<stdio.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include"common.h"

void main()
{
	char w_buf[100]="hello Anvitha";
	char r_buf[100];
	int fd;
	fd=open("/proc/proc_basic",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return;
	}

	printf("string before witing into kernel: %s\n",w_buf);
	write(fd,w_buf,strlen(w_buf)+1);

	read(fd,r_buf,sizeof(r_buf));
	printf("string from the kernel space: %s\n",r_buf);

}
