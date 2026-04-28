#include<stdio.h>
void main()
{
	int r,c,i,j;
	printf("Enter rows and columns:\n");
	scanf("%d%d",&r,&c);
	int a[r][c];
	int *p=&a[0][0];
	printf("Enter array elements:\n");
	for(i=0;i<r;i++)
	{
		for(j=0;j<c;j++)
		{
			scanf("%d",(p+i*c+j));
		}
	}
	for(i=0;i<r;i++)
	{
		for(j=0;j<c;j++)
		{
			printf("%d ",*(p+i*c+j));
		}
		printf("\n");
	}
}

/*#include<stdio.h>
void main()
{
	int r,c,i,j;
	printf("Enter rows and columns:\n");
	scanf("%d%d",&r,&c);
	int a[r][c];
	int (*p)[c]=a;
	printf("Enter array elements:\n");
	for(i=0;i<r;i++)
	{
		for(j=0;j<c;j++)
		{
			scanf("%d",&p[i][j]);
		}
	}
	for(i=0;i<r;i++)
	{
		for(j=0;j<c;j++)
			printf("%d ",p[i][j]);
		printf("\n");
	}
}*/

/*#include<stdio.h>
#include<stdlib.h>
void main()
{
	int r,c,i,j;
	printf("Enter number of rows and columns:\n");
	scanf("%d%d",&r,&c);
	int **p=malloc(sizeof(int*)*r);
	for(i=0;i<r;i++)
		p[i]=malloc(sizeof(int)*c);
	printf("Enter array elements:\n");
	for(i=0;i<r;i++)
	{
		for(j=0;j<c;j++)
		{
			scanf("%d",&p[i][j]);
		}
	}

	for(i=0;i<r;i++)
	{
		for(j=0;j<c;j++)
			printf("%d ",p[i][j]);
		printf("\n");
	}
}*/
