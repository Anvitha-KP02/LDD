#include <stdio.h>
#include <string.h>

int main()
{
    char str1[20] = "abcdef";
    char str2[20] = "abcdef";

    /* Using memcpy (overlapping memory) */
    memcpy(str1 + 2, str1, 4);

    /* Using memmove (handles overlap) */
    memmove(str2 + 2, str2, 4);

    printf("After memcpy  : %s\n", str1);
    printf("After memmove : %s\n", str2);

    return 0;
}

/*#include <stdio.h>
#include <string.h>

int main()
{
    char str[] = "abcdef";

    memcpy(str+2, str, 4);

    printf("%s", str);
}*/

/*#include <stdio.h>
#include <string.h>

int main() {
    char src[] = "Hello, World!";
    char dest[20];

    memcpy(dest, src, strlen(src) + 1);
    printf("Copied string: %s\n", dest);

    return 0;
}
*/
