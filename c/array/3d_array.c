// 3d array using pointer to an array

#include<stdio.h>
void main()
{
	int i,j,k,x,y,z;
	printf("Enter the number of block, number of rwos and number of columns:\n");
	scanf("%d%d%d",&x,&y,&z);
	int a[x][y][z];
	int (*p)[y][z]=a;
	printf("Enter the array elements:\n");
	for(i=0;i<x;i++)
	{
		for(j=0;j<y;j++)
		{
			for(k=0;k<z;k++)
			{
				scanf("%d",&p[i][j][k]);   // scanf just needs an address not value
			}
		}
	}

	for(i=0;i<x;i++)
	{
		for(j=0;j<y;j++)
		{
			for(k=0;k<z;k++)
			{
				printf("%d ",p[i][j][k]);
			}
			printf("\n");
		}
		printf("\n");
	}
}

// 3d array using normal pointer

/*#include<stdio.h>
void main()
{
	int i,j,k,x,y,z;
	printf("Enter the number of block, number of rows and number of columns:\n");
	scanf("%d%d%d",&x,&y,&z);
	int a[x][y][z];
	int *p=&a[0][0][0];
	printf("Enter the array elements:\n");
	for(i=0;i<x;i++)
	{
		for(j=0;j<y;j++)
		{
			for(k=0;k<z;k++)
			{
				scanf("%d",(p + i*y*z + j*z + k));   // scanf just needs an address not value
			}
		}
	}

	for(i=0;i<x;i++)
	{
		for(j=0;j<y;j++)
		{
			for(k=0;k<z;k++)
			{
				printf("%d ",*(p + i*y*z + j*z + k));
			}
			printf("\n");
		}
		printf("\n");
	}
}*/


/*#include<stdio.h>
#include<stdlib.h>
void main()
{
	int i,j,k,x,y,z;
	printf("Enter the number of block, number of rows and number of columns:\n");
	scanf("%d%d%d",&x,&y,&z);
	int *p=malloc(sizeof(int)*x*y*z);
	printf("Enter the array elements:\n");
	for(i=0;i<x;i++)
	{
		for(j=0;j<y;j++)
		{
			for(k=0;k<z;k++)
			{
				scanf("%d",(p+i*y*z+j*z+k));   // scanf just needs an address not value
			}
		}
	}

	for(i=0;i<x;i++)
	{
		for(j=0;j<y;j++)
		{
			for(k=0;k<z;k++)
			{
				printf("%d ",*(p+i*y*z+j*z+k));
			}
			printf("\n");
		}
		printf("\n");
	}
}*/

// allocating memory separately for x,y,z
/*#include<stdio.h>
#include<stdlib.h>
void main()
{
	int x,y,z,i,j,k;

	printf("Enter x,y,z\n");
	scanf("%d%d%d",&x,&y,&z);

	int ***p=malloc(sizeof(int**)*x);

	for(i=0;i<x;i++)
	{
		p[i]=malloc(sizeof(int*)*y);
		for(j=0;j<y;j++)
		{
			p[i][j]=malloc(sizeof(int)*z);
		}
	}

	printf("Enter array elements:\n");
	for(i=0;i<x;i++)
	{
		for(j=0;j<y;j++)
		{
			for(k=0;k<z;k++)
			{
				scanf("%d",&p[i][j][k]);
			}
		}
	}
	for(i=0;i<x;i++)
	{
		for(j=0;j<y;j++)
		{
			for(k=0;k<z;k++)
			{
				printf("%d ",p[i][j][k]);
			}
			printf("\n");
		}
		printf("\n");
	}
}*/
