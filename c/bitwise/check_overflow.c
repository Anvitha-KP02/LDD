#include <stdio.h>
#include <stdint.h>

int main()
{
    uint8_t a, b, result;

	printf("Enter a and b\n");
	scanf("%hhu%hhu",&a,&b);
    result = a + b;

    if(result < a)
        printf("Overflow\n");
    else
        printf("No Overflow\n");

    printf("Result = %u\n", result);

    return 0;
}

/*   case1: No overflow

a=12, b=15
result=12+15=27
result<a  ==>  27<12 -> false (NO overflow)


case 2: Overflow

a=250, b=10
result=250+10 ====>  260   
uint8_t can store only upto 255
so, 260%256  = 4
result=4

result<a   ===>   4<250   -> true(Overflow)

*/



/*#include<stdio.h>
#include<stdint.h>
void main()
{
uint8_t a,b;
a=12;
b=15;
//printf("Enter a and b\n");
//scanf("%d%d",&a,&b);
if((a^b)>0)
printf("Overflow\n");
else
printf("Not an Overflow\n");
}*/


/*#include<stdio.h>
#include<stdint.h>
void main()
{
uint8_t a,b;
a=128;
b=128;
//printf("Enter a and b:\n");
//scanf("%d%d",&a,&b);
if((a+b)>255)
printf("Overflow\n");
else
printf("Not an overflow\n");
}*/
