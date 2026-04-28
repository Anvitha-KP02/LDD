#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const char *filename = "sample.txt";
    FILE *fp = NULL;
    char buffer[100];

    /* Step 1: Open file in write mode ("w") */
    fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Error opening file for writing");
        return EXIT_FAILURE;
    }

    /* Step 2: Write "hello" into the file */
    if (fprintf(fp, "hello") < 0) {
        perror("Error writing to file");
        fclose(fp);
        return EXIT_FAILURE;
    }

    /* Step 3: Close file after writing */
    fclose(fp);

    /* Step 4: Reopen the same file in read mode ("r") */
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file for reading");
        return EXIT_FAILURE;
    }

    /* Step 5: Read file content and print it */
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("File content: %s\n", buffer);
    } else {
        printf("File is empty or read failed.\n");
    }

    /* Step 6: Close file after reading */
    fclose(fp);

    return EXIT_SUCCESS;
}
