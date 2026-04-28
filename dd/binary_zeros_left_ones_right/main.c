/*
 * Rearrange a binary array: all 0s on the left, all 1s on the right.
 * In-place, O(n), no sorting API.
 */

#include <stdio.h>

#define N 10

static void move_zeros_left(int *a, int n)
{
	int insert = 0;
	int i;
	int t;

	for (i = 0; i < n; i++) {
		if (a[i] == 0) {
			t = a[insert];
			a[insert] = a[i];
			a[i] = t;
			insert++;
		}
	}
}

static void print_array(const int *a, int n)
{
	int i;

	printf("{");
	for (i = 0; i < n; i++) {
		printf("%d", a[i]);
		if (i + 1 < n)
			printf(",");
	}
	printf("}\n");
}

int main(void)
{
	int a[N] = {1, 0, 1, 1, 0, 0, 0, 1, 1, 0};

	printf("Before: ");
	print_array(a, N);

	move_zeros_left(a, N);

	printf("After:  ");
	print_array(a, N);

	return 0;
}
