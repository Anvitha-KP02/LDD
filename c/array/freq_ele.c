#include<stdio.h>
void main()
{
	int a[10],freq[256]={0},ele,i;
	printf("Enter the number array elements:\n");
	scanf("%d",&ele);
	printf("Enter array elements:\n");
	for(i=0;i<ele;i++)
		scanf("%d",&a[i]);
	int *p=a;
	for(i=0;i<ele;i++)
		freq[*(p+i)]++;
	for(i=0;i<256;i++)
	{
		if(freq[i]!=0)
			printf("%d ----> %d times\n",i,freq[i]);
	}
}

/*#include<stdio.h>
int main()
{
    int a[6], freq[100] = {0};
    int i, ele = 6;

    printf("Enter the array elements:\n");
    for (i = 0; i < ele; i++)
        scanf("%d", &a[i]);

    // single loop 
    for (i = 0; i < ele; i++)
        freq[a[i]]++;

    // printing result 
    for (i = 0; i < 100; i++)
    {
        if (freq[i] != 0)
            printf("%d  ------>  %d times\n", i, freq[i]);
    }

    return 0;
}*/


/*#include<stdio.h>
void main()
{
	int a[6],ele,i,j,k,c;
	ele=sizeof(a)/sizeof(a[0]);
	printf("Enter the array elements:\n");
	for(i=0;i<ele;i++)
		scanf("%d",&a[i]);
	for(i=0;i<ele;i++)
	{
		for(j=i+1,c=1;j<ele;j++)
		{
			if(a[i]==a[j])
			{
				c++;
				for(k=j;k<ele-1;k++)
				{
					a[k]=a[k+1];
				}
				j--;
				ele--;
			}
		}
		printf("%d  ------>  %d times\n",a[i],c);
	}
}*/

