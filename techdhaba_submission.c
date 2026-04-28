/*
TECHDHABA_SUBMISSION
NAME: <Anvitha K P>
EMAIL: <anvitha@mirafra.com>
PROBLEM_ID: P1 | P2 | P3 | ... | P12
DATE: 2026-02-06
*/

#include<stdio.h>
#include<stdint.h>
struct data
{
uint32_t op;
int32_t a;
int32_t b;
};

//p1
uint32_t frame;
uint32_t action;

uint32_t new_frame;
uint32_t status;
int toggle(uint32_t,uint32_t);
void print_binary(int);

struct data d;
int32_t result;
int add(int a,int b)
{
return a+b;
}
int sub(int a,int b)
{
return a-b;
}
int (*select_operation())(int,int)
{
int (*ops[])(int,int)={NULL,add,sub};
printf("Enter option 1) add 2) sub:\n");
scanf("%d",&d.op);
return ops[d.op];;
}
void main()
{
printf("Enter a, b:\n");
scanf("%d%d",&d.a,&d.b);
int (*fptr)(int,int)=select_operation();
result=fptr(d.a,d.b);
printf("Answer=%d\n",result);

//p3
printf("Enter the frame and action:\n");
scanf("%d%d",&frame,&action);
int bit_ret_val=toggle(frame,action);
print_binary(frame);
if(bit_ret_val!=0)
printf("Not equal\n");
else
printf("Same\n");
}

void print_binary(int num)
{
int pos;
for(pos=31;pos>=0;pos--)
{
printf("%d",num>>pos&1);
}
printf("\n");
}

int toggle(uint32_t num,uint32_t pos)
{
new_frame=num^(1<<pos);
printf("new_frame=%d\n",new_frame);
print_binary(new_frame);
int b1,b2;
b1=new_frame>>pos&1;
b2=new_frame>>2&1;
if(b1!=b2)
return -1;
else
return 0;
}
