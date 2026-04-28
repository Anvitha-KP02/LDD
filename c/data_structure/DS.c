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
void add_node_end(emp**);
void insert_at_position(emp**);
void delete_all_nodes(emp**);
void delete_first_node(emp**);
void delete_last_node(emp**);
void delete_particular_node(emp**);
int count_num_nodes(emp*);
void add_middle(emp**);
void reverse_print(emp*);
void delete_nth_from_last(emp **);
void print_nth_from_last(emp*);
void reverse_list(emp**);
int searchKey(emp*,int);

void main()
{
	emp *hptr=0;
	int op,c;
	while(1)
	{
	printf("Enter the option:\n");
	printf("Enter 1)create_node 2)print_node 3)add_node_end 4)insert_at_position 5)delete_all_node 6)delete_first_node 7)delete_last_node 8)delete_particular_node 9)count_num_nodes 10)add_middle 11)reverse_print 12)delete_node_last 13)print_nth_from_last 14)reverse_list 15)searchKey 16)exit\n");
	scanf("%d",&op);
	switch(op)
	{
		case 1: create_node_begin(&hptr);
			break;
		case 2: print_node(hptr);
			break;
		case 3: add_node_end(&hptr);
			break;
		case 4: insert_at_position(&hptr);
			break;
		case 5: delete_all_nodes(&hptr);
			break;
		case 6: delete_first_node(&hptr);
			break;
		case 7: delete_last_node(&hptr);
			break;
		case 8: delete_particular_node(&hptr);
			break;
		case 9: c=count_num_nodes(hptr);
			printf("Number of nodes are: %d\n",c);
			break;
		case 10: add_middle(&hptr);
			break;
		case 11: reverse_print(hptr);
			break;
		case 12: delete_nth_from_last(&hptr);
			break; 
		case 13: print_nth_from_last(hptr);
			break;
		case 14: reverse_list(&hptr);
			break;
		case 15:int key;
			printf("Enter the key value:\n");
			scanf("%d",&key); 
			if(searchKey(hptr,key))
			printf("Key found\n");
			else
			printf("Key not found\n");
		case 16: exit(0);
		default: printf("Unknown option\n");
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

void add_node_end(emp **ptr)
{
	emp *new=malloc(sizeof(struct employee));
	printf("Enter the name, id and salary:\n");
	scanf("%s%d%f",new->name,&new->emp_id,&new->salary);
	new->next=0;
	if(*ptr==0)
		*ptr=new;
	else
	{
		emp *last=*ptr;
		while(last->next)
			last=last->next;
		last->next=new;
	}
}

void insert_at_position(emp **ptr)
{	
	emp *new=malloc(sizeof(struct employee));
	int pos;
	printf("Enter the name,id and salary to be inserted:\n");
	scanf("%s%d%f",new->name,&new->emp_id,&new->salary);
	printf("Enter the pos where new node to be inserted:\n");
	scanf("%d",&pos);
	
	if(pos<1)
	{
		printf("Invalid operation:\n");
		return;
	}

	if(pos==1)
	{
		new->next=*ptr;
		*ptr=new;
		return;
	}

	emp *temp=*ptr;

	for(int i=1;i<pos-1;i++)
	{
		if(temp==NULL)
		{
			printf("Position out of range:\n");
			free(new);
			return;
		}
		temp=temp->next;
	}

	new->next=temp->next;
	temp->next=new;
}

void delete_all_nodes(emp **ptr)
{
	int c=1;
	if(*ptr==0)
	{
		printf("No records found\n");
		return;
	}
	emp *del=*ptr;
	while(del)
	{
		*ptr=del->next;
		free(del);
		printf("Node deleted: %d\n",c++);
		sleep(1);
		del= *ptr;
	}
	printf("All nodes are deleted\n");
}

void delete_first_node(emp **ptr)
{
	if(*ptr==0)
	{
		printf("No records found\n");
		return;
	}

	emp *del=*ptr;
	if(del)
	{
		*ptr=del->next;
		free(del);
		printf("First node is deleted\n");
	}
}

void delete_last_node(emp **ptr)
{
	if(*ptr==0)
	{
		printf("No records found\n");
		return;
	}

	emp *del=*ptr;
	if(del->next==NULL)
	{
		free(del);
		return;
	}

	while(del->next->next)
	{
		del=del->next;
	}
	free(del->next);
	del->next=NULL;
}

void delete_particular_node(emp **ptr)
{
	if(*ptr==0)
	{
		printf("No nodes are there to delete\n");
		return;
	}
	int id;
	printf("Enter the employee id to delete\n");
	scanf("%d",&id);

	emp *cur=*ptr;
	emp *prev=NULL;

	while(cur!=NULL && cur->emp_id!=id)
	{
		prev=cur;
		cur=cur->next;
	}
	if(cur==NULL)
	{
		printf("Id is not found\n");
		return;
	}
	if(prev==NULL)
	{
		*ptr=cur->next;
	}
	else
	{
		prev->next=cur->next;
	}
	free(cur);
	printf("The particular node is deleted\n");
}


int count_num_nodes(emp *ptr)
{
	int c=0;
	while(ptr)
	{
		c++;
		ptr=ptr->next;
	}
	return c;
}

void add_middle(emp **ptr)
{
	emp *new=malloc(sizeof(struct employee));
	printf("Enter the emp_name, emp_id and salary:\n");
	scanf("%s%d%f",new->name,&new->emp_id,&new->salary);
	if(*ptr==0 || (new->emp_id)<((*ptr)->emp_id))
	{
		new->next=*ptr;
		*ptr=new;
	}
	else
	{
		emp *last=*ptr;
		while(last->next!=0 && (new->emp_id)>(last->next->emp_id))
			last=last->next;
		new->next=last->next;
		last->next=new;
	}
}

void reverse_print(emp *ptr)  // it is slow bcz, count nodes-->o(n), outer loop---> o(n) ,inner loop--->o(n)  so total complexity is o(n^2)
{
	if(ptr==0)
	{
		printf("No records found:\n");
		return;
	}

	emp *t;
	int i,j,c;
	c=count_num_nodes(ptr);
	for(i=0;i<c;i++)
	{
		t=ptr;
		for(j=0;j<c-i-1;j++)
			t=t->next;
		printf("Name=%s emp_id=%d salary=%f\n",t->name,t->emp_id,t->salary);
	}
}

/*void reverse_print(emp *ptr)    // using recursion, time complextity is o(n)
  {
  if(ptr == NULL)
  {
  return;
  }

  reverse_print(ptr->next);

  printf("Name=%s emp_id=%d salary=%f\n",
  ptr->name, ptr->emp_id, ptr->salary);
  }*/

/*void delete_nth_from_last(emp **ptr)
{
	if(*ptr==0)
	{
		printf("No records found\n");
		return;
	}

	int n;
	printf("Enter the value of n(nth node from last):\n");
	scanf("%d",&n);

	int total=count_num_nodes(*ptr);

	if(n<=0 || n>total)
	{
		printf("Invalid position\n");
		return;
	}
	int pos_from_start=total-n+1;
	emp *cur=*ptr;
	emp *prev=NULL;

	if(pos_from_start==1)
	{
		*ptr=cur->next;
		free(cur);
		printf("Nth node from last deleted\n");
		return;
	}

	for(int i=1;i<pos_from_start;i++)
	{
		prev=cur;
		cur=cur->next;
	}
	prev->next=cur->next;
	free(cur);

	printf("Nth node from last deleted\n");
}*/

void delete_nth_from_last(emp **ptr)
{
	if(*ptr==0)
	{
		printf("No records found\n");
		return;
	}

	int n,i;
	printf("Enter the value of n(nth node from last):\n");
	scanf("%d",&n);

	emp *fast=*ptr;
	emp *slow=*ptr;
	emp *prev=NULL;

	// Move fast ptr n steps ahead
	for(i=0;i<n;i++)  /// here moving the fast pointer 2 steps
	{
		if(fast==NULL)
		{
			printf("Position exceeded number of nodes\n");
			return;
		}
		fast=fast->next;
	}

	// Move both the pointers
	while(fast!=NULL)
	{
		prev=slow;
		slow=slow->next;
		fast=fast->next;
	}           /// when fast becomes NULL, the slow will be at Nth node from last
	
	if(prev==NULL)
	{
	*ptr=slow->next;
	}
	else
	{
	prev->next=slow->next;
	}
	free(slow);
	printf("Nth node from last deleted\n");
//printf("Name: %s, emp_id=%d, salary=%f\n",slow->name,slow->emp_id,slow->salary);
}         /// Time complexity is O(n), and the traversal is 1


void print_nth_from_last(emp *ptr)
{
	if(ptr==0)
	{
		printf("No records found\n");
		return;
	}

	int n,i;
	printf("Enter the value of n(nth node from last):\n");
	scanf("%d",&n);

	emp *fast=ptr;
	emp *slow=ptr;

	// Move fast ptr n steps ahead
	for(i=0;i<n;i++)  /// here moving the fast pointer 2 steps
	{
		if(fast==NULL)
		{
			printf("Position exceeded number of nodes\n");
			return;
		}
		fast=fast->next;
	}

	// Move both the pointers
	while(fast!=NULL)
	{
		slow=slow->next;
		fast=fast->next;
	}           /// when fast becomes NULL, the slow will be at Nth node from last

	printf("Nth node from last details:\n");
	printf("Name: %s, emp_id=%d, salary=%f\n",slow->name,slow->emp_id,slow->salary);
}         /// Time complexity is O(n), and the traversal is 1

/*void print_nth_from_last(emp *ptr)
  {
  if(ptr == 0)
  {
  printf("No records found\n");
  return;
  }

  int n;
  printf("Enter the value of N (nth node from last):\n");
  scanf("%d",&n);

  int total = count_num_nodes(ptr);

*ptr=cur->next;
242         }
  if(n <= 0 || n > total)
  {
  printf("Invalid position\n");
  return;
  }

  int pos_from_start = total - n + 1;

  emp *temp = ptr;

  for(int i = 1; i < pos_from_start; i++)
  {
  temp = temp->next;
  }

  printf("Nth node from last details:\n");
  printf("Name=%s emp_id=%d salary=%f\n",
  temp->name, temp->emp_id, temp->salary);
  }*/

void reverse_list(emp **ptr)
{
	emp *prev=NULL;
	emp *cur=*ptr;
	emp *next=NULL;

	while(cur)
	{
		next=cur->next;
		cur->next=prev;
		prev=cur;
		cur=next;
	}
	*ptr=prev;
}

int searchKey(emp *ptr,int key)
{
emp *cur=ptr;
while(cur!=NULL)
{
if(cur->emp_id==key)
return 1;
cur=cur->next;
}
return 0;
}

