#include<stdio.h>
#include<stdlib.h>
typedef struct Node
{
	int data;
	struct Node *next;
}node;

void create_node(node **ptr);
void print_node(node *ptr);
int iscircular(node *ptr);
void create_circular_list(node **ptr);
void print_node_circular(node *ptr);

void main()
{
	node *hptr=0;
	int op;
	while(1)
	{
	printf("Enter 1)create_node 2)print_node 3)create_circular_list 4)check_linear_circular 5)print_node_circular 6) exit\n");
	scanf("%d",&op);
		switch(op)
		{
			case 1: create_node(&hptr);
				break;
			case 2: print_node(hptr);
				break;
			case 3: create_circular_list(&hptr);
				break;
			case 4: if(iscircular(hptr))
					printf("Linked list is circular\n");
				else
					printf("Not circular\n");
				break;
			case 5: print_node_circular(hptr);
				break;
			case 6: exit(0);
			default: printf("Invalid option\n");
		}
	}
}

void create_node(node **ptr)
{
	node *new=malloc(sizeof(struct Node));
        printf("Enter the name, employee id and salary:\n");
        scanf("%d",&new->data);
        new->next=*ptr;
        *ptr=new;
}

void print_node(node *ptr)
{
        if(ptr==0)
        {
                printf("No records found:\n");
                return;
        }

        while(ptr)
        {
                printf("Data=%d\n",ptr->data);
                ptr=ptr->next;
        }
}

int iscircular(node *ptr)
{
    node *slow = ptr;
    node *fast = ptr;

    while (fast != NULL && fast->next != NULL)
    {
        slow = slow->next;          // move 1 step
        fast = fast->next->next;    // move 2 steps

        if (slow == fast)
            return 1;   // Circular
    }
    return 0;           // Linear
}

void create_circular_list(node **ptr)
{
    node *new, *temp;

    new = malloc(sizeof(node));
    printf("Enter data:\n");
    scanf("%d", &new->data);
    new->next = NULL;

    // If list is empty
    if (*ptr == NULL)
    {
        *ptr = new;
        new->next = new;   // Point to itself (single node circular)
        return;
    }

    // Traverse to last node
    temp = *ptr;
    while (temp->next != *ptr)
    {
        temp = temp->next;
    }

    // Insert new node at end and make it circular
    temp->next = new;
    new->next = *ptr;
}

void print_node_circular(node *ptr)
{
    if (ptr == NULL)
    {
        printf("No records found\n");
        return;
    }

    if (iscircular(ptr))
    {
        node *start = ptr;
        do
        {
            printf("Data=%d\n", ptr->data);
            ptr = ptr->next;
        } while (ptr != start);
    }
    else
    {
        while (ptr != NULL)
        {
            printf("Data=%d\n", ptr->data);
            ptr = ptr->next;
        }
    }
}

// Detect the loop and print from which node the loop get started.

/*

emp *fast=*ptr;
emp *slow=*ptr;
int c=1;
while(fast!=NULL && fast->next!=NULL)
{
slow=slow->next;
fast=fast->next->next;
if(slow==fast)
{
emp* second=*ptr;
while(slow!=second)
{
second=second->next;
slow=slow->next;
c++;
}
}
}

*/
