
#include<stdio.h>
void main()
{
int x;

printf("%ld %ld",&x,&x+1);
printf("Sizeof of int %ld\n",(char*)(&x+1)-(char *)&x);
}

/*#include<stdio.h>
void main()
{
float *p=0;
printf("Sizeof of int %ld\n",(char*)(p+1)-(char*)p);
}*/

/*#include<stdio.h>
void main()
{
char *p=0;
printf("Sizeof of int %ld\n",(p+1)-p);
}*/

/*#include<stdio.h>
void main()
{
int *p=0;
printf("Sizeof of int %ld\n",(char*)(p+1)-(char*)p);
}*/
