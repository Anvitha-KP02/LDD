#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<unistd.h>

int main()
{
const char* file_path = "GFG-M-Mapping.txt";
const size_t file_size = 4096;

int fd=open(file_path, O_RDWR | O_CREAT, 0400 | 0200);
if(fd==-1)
{
perror("open");
return 1;
}

if(ftruncate(fd,file_size)==-1)
{
perror("ftruncate");
close(fd);
return 1;
}

void* file_memory=mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
if(file_memory == MAP_FAILED)
{
perror("mmap");
close(fd);
return 1;
}

const char* message="Hello Anvitha!";

strncpy(file_memory, message, strlen(message));

printf("Contents of the memory-mapped region: %s\n",(char*)file_memory);

munmap(file_memory, file_size);
close(fd);
return 0;
}
