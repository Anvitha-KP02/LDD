#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int IO0DIR_ADDR;
unsigned int IO0SET_ADDR;
unsigned int IO0CLR_ADDR;
unsigned int T0TCR_ADDR;
unsigned int T0PR_ADDR;

void load_addresses(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        exit(1);
    }

    char name[20];
    unsigned int addr;

    while (fscanf(fp, "%s %x", name, &addr) != EOF) {
        if (strcmp(name, "IO0DIR") == 0)
            IO0DIR_ADDR = addr;
        else if (strcmp(name, "IO0SET") == 0)
            IO0SET_ADDR = addr;
        else if (strcmp(name, "IO0CLR") == 0)
            IO0CLR_ADDR = addr;
        else if (strcmp(name, "T0TCR") == 0)
            T0TCR_ADDR = addr;
        else if (strcmp(name, "T0PR") == 0)
            T0PR_ADDR = addr;
    }

    fclose(fp);
}

int main()
{
    load_addresses("addr.txt");

    volatile unsigned int *IO0DIR = (unsigned int *)IO0DIR_ADDR;
    volatile unsigned int *IO0SET = (unsigned int *)IO0SET_ADDR;
    volatile unsigned int *IO0CLR = (unsigned int *)IO0CLR_ADDR;

    printf("IO0DIR = 0x%X\n", IO0DIR_ADDR);
    printf("IO0SET = 0x%X\n", IO0SET_ADDR);
    printf("IO0CLR = 0x%X\n", IO0CLR_ADDR);
    printf("T0TCR = 0x%X\n", T0TCR_ADDR);
    printf("T0PR = 0x%X\n", T0PR_ADDR);
    return 0;
}



/*#include "addr.c"
extern volatile unsigned int *IO0DIR;
extern volatile unsigned int *IO0SET;
extern volatile unsigned int *IO0CLR;
extern volatile unsigned int *T0TCR;
extern volatile unsigned int *T0PR;

void delay(unsigned int d)
{
    *T0TCR = 0x02;     // reset timer
    *T0PR  = 14999999;
    *T0TCR = 0x01;     // start timer
    while (*T0TCR < d);
}

int main()
{
    *IO0DIR |= (1 << 0);   // P0.0 output

    *IO0CLR = (1 << 0);
    delay(10);
    *IO0SET = (1 << 0);

    while (1);
}
*/


/*#define IO0DIR (*(volatile unsigned int*)0xE0028008)
#define IO0SET (*(volatile unsigned int*)0xE0028004)
#define IO0CLR (*(volatile unsigned int*)0xE002800C)
#define T0TCR (*(volatile unsigned int*)0xE0028004)
#define T0PR (*(volatile unsigned int*)0xE002800C)
void delay(unsigned int)
{
	T0TCR=1;
	T0PR=14999999;
	T0TCR=0;
}

int main()
{
	IO0DIR=1<<0;
	IO0CLR=1<<0;
	delay(10);
	IO0SET=1<<0;
}*/

//customized to hardware, no translation is happening
