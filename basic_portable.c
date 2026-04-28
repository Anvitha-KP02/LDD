/*
 * basic_portable.c
 *
 * Linux compile:
 *   gcc basic_portable.c -o basic_portable
 *
 * Windows compile (MSVC):
 *   cl basic_portable.c
 */

#include <stdio.h>

/* Detect OS */
#if defined(_WIN32) || defined(_WIN64)
    #define OS_NAME "Windows"
#else
    #define OS_NAME "Linux/Unix"
#endif

/* Detect Architecture */
#if defined(_x86_64_) || defined(_M_X64)
    #define ARCH_NAME "x86_64 (64-bit)"
#elif defined(_i386_) || defined(_M_IX86)
    #define ARCH_NAME "x86 (32-bit)"
#elif defined(_aarch64_) || defined(_M_ARM64)
    #define ARCH_NAME "ARM64 (64-bit)"
#elif defined(_arm_) || defined(_M_ARM)
    #define ARCH_NAME "ARM (32-bit)"
#else
    #define ARCH_NAME "Unknown Architecture"
#endif

int main(void)
{
    printf("Hello from Portable C Program!\n");
    printf("OS   : %s\n", OS_NAME);
    printf("ARCH : %s\n", ARCH_NAME);

    printf("Pointer Size: %zu bytes\n", sizeof(void*));
    return 0;
}
