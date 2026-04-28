#include <stdio.h>

int main() {
    // This will cause a compilation error or warning
    register volatile int status = 0; 

    printf("Status: %d\n", status);
    
    return 0;
}


/*#include<stdio.h>
void main()
{
register int num=10;
printf("value=%d\n",num);
printf("Address=%p\n",&num);
}*/
