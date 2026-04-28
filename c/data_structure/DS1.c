#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

typedef struct employee
{
	char name[50];
	int emp_id;
	float salary;
	struct employee *next;
}emp;

void create_node_begin(emp**);
void print_node(emp*);
void move_lstnode_fst(emp**);
void middlenode_print(emp**);
void delete_altnode(emp**);
void rearrange_even_odd(emp**);

void main()
{
	emp *hptr=0;
	int op,c;
	while(1)
	{
	printf("Enter the option:\n");
	printf("Enter 1)create_node 2)print_node 3)move_lastnode_fst 4)middlenode_print 5)delete alternate nodes 6)rearrange_even_odd 7)exit \n");
	scanf("%d",&op);
	switch(op)
	{
		case 1: create_node_begin(&hptr);
			break;
		case 2: print_node(hptr);
			break;
		case 3: move_lstnode_fst(&hptr);
			break;
		case 4: middlenode_print(&hptr);
			break;
		case 5: delete_altnode(&hptr);
			break;
		case 6: rearrange_even_odd(&hptr);
			break;
		case 7: exit(0);
	}
	}
}

void create_node_begin(emp **ptr)
{
	emp *new=malloc(sizeof(struct employee));
	printf("Enter the name, employee id and salary:\n");
	scanf("%s%d%f",new->name,&new->emp_id,&new->salary);
	new->next=*ptr;
	*ptr=new;
}

void print_node(emp *ptr)
{
	if(ptr==0)
	{
		printf("No records found:\n");
		return;
	}

	while(ptr)
	{
		printf("Name=%s emp_id=%d salary=%f\n",ptr->name,ptr->emp_id,ptr->salary);
		ptr=ptr->next;
	}
}

void move_lstnode_fst(emp **ptr)
{
	emp *cur=*ptr;
	emp *prev=NULL;

	while(cur->next!=NULL)
	{
		prev=cur;
		cur=cur->next;
	}

	cur->next=*ptr;
	*ptr=cur;
	prev->next=NULL;
}

void middlenode_print(emp **ptr)
{
	if(*ptr==NULL || (*ptr)->next==NULL)
		return;

	emp *fast=*ptr;
	emp *slow=*ptr;
	emp *prev=NULL;
	while(fast!=NULL && fast->next!=NULL)
	{
		prev=slow;
		slow=slow->next;
		fast=fast->next->next;
	}

	printf("%s %d %f\n",slow->name,slow->emp_id,slow->salary);
	prev->next=slow->next;
	slow->next=*ptr;
	*ptr=slow;
}

void delete_altnode(emp **ptr)
{
	emp *cur=*ptr;
	emp *del;
	while(cur!=NULL && cur->next!=NULL)
	{
		del=cur->next;
		cur->next=del->next;
		free(del);
		cur=cur->next;
	}
}

void rearrange_even_odd(emp **ptr)
{
	emp *odd=*ptr;
	emp *even=odd->next;
	emp *evenHead=even;
	while(even!=NULL && even->next!=NULL)
	{
		odd->next=odd->next->next;
		odd=odd->next;

		even->next=even->next->next;
		even=even->next;
	}
	odd->next=evenHead;
}
