#include <stdio.h>
#include <string.h>

int main() {
    char str[] = "Hello, World!";
    printf("Original string: %s\n", str);

    memmove(str + 7, str + 5, 7);
    printf("Modified string: %s\n", str);

    return 0;
}

