
void main()
{
	int a[]={1,2,3,4,5,6,7},ele,i,j,temp,k;
	ele=sizeof(a)/sizeof(a[0]);
	printf("Enter number of rotations:\n");
	scanf("%d",&k);
	for(i=0;i<=k;i++)
	{
		temp=a[0];
		for(j=0;j<ele;j++)
		{
			a[j]=a[j+1];
		}
		a[ele-1]=temp;
	}
	for(i=0;i<ele;i++)
		printf("%d ",a[i]);
	printf("\n");
}

	
