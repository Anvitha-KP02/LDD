#include <stdio.h>
#include <string.h>

// Function to check if character is alphabet
int is_alpha(char ch)
{
    if((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
        return 1;
    return 0;
}

// Convert uppercase to lowercase
char to_lower(char ch)
{
    if(ch >= 'A' && ch <= 'Z')
        return ch + 32;
    return ch;
}

// Function to check anagram (optimized)
int is_anagram(char s1[], char s2[])
{
    int freq[26] = {0};
    int i;

    // Traverse both strings
    for(i = 0; s1[i] || s2[i]; i++)
    {
        // Process s1
        if(s1[i])
        {
            if(!is_alpha(s1[i]))
                return -1; // special character found

            char ch1 = to_lower(s1[i]);
            freq[ch1 - 'a']++;
        }

        // Process s2
        if(s2[i])
        {
            if(!is_alpha(s2[i]))
                return -1; // special character found

            char ch2 = to_lower(s2[i]);
            freq[ch2 - 'a']--;
        }
    }

    // Check frequency array
    for(i = 0; i < 26; i++)
    {
        if(freq[i] != 0)
            return 0;
    }

    return 1;
}

int main()
{
    char s1[50], s2[50];

    printf("Enter string1:\n");
    scanf("%[^\n]", s1);

    printf("Enter string2:\n");
    scanf(" %[^\n]", s2);

    int result = is_anagram(s1, s2);

    if(result == -1)
        printf("Special character found in input\n");
    else if(result == 1)
        printf("Anagram strings\n");
    else
        printf("Not an anagram\n");

    return 0;
}

/*#include <stdio.h>
#include <string.h>

// Function to sort a string (Bubble Sort)
void sort_string(char s[])
{
    int i, j;
    char temp;

    for(i = 0; s[i]; i++)
    {
        for(j = 0; s[j+1]; j++)
        {
            if(s[j] > s[j+1])
            {
                temp = s[j];
                s[j] = s[j+1];
                s[j+1] = temp;
            }
        }
    }
}

// Function to check anagram
int is_anagram(char s1[], char s2[])
{
    int l1 = strlen(s1);
    int l2 = strlen(s2);

    // Step 1: Length check
    if(l1 != l2)
        return 0;

    // Step 2: Sort both strings
    sort_string(s1);
    sort_string(s2);

    // Step 3: Compare
    if(strcmp(s1, s2) == 0)
        return 1;
    else
        return 0;
}

int main()
{
    char s1[50], s2[50];

    printf("Enter string1:\n");
    scanf("%[^\n]", s1);

    printf("Enter string2:\n");
    scanf(" %[^\n]", s2);

    if(is_anagram(s1, s2))
        printf("Anagram strings\n");
    else
        printf("Not an anagram\n");

    return 0;
}*/

/*#include<stdio.h>
#include<string.h>
void main()
{
char s1[50],s2[50];
printf("Enter string1:\n");
scanf("%[^\n]",s1);
printf("Enter string2:\n");
scanf(" %[^\n]",s2);
int i,j;
int l1,l2;
for(l1=0;s1[l1];l1++);
for(l2=0;s2[l2];l2++);
if(l1!=l2)
{
printf("Two strings are not anagram\n");
return;
}

for(i=0;i<l1-1;i++)
{
for(j=0;j<l1-1-i;j++)
{
if(s1[j]>s1[j+1])
{
char t=s1[j];
s1[j]=s1[j+1];
s1[j+1]=t;
}
}
}
//printf("%s\n",s1);

for(i=0;i<l2-1;i++)
{
for(j=0;j<l2-1-i;j++)
{
if(s2[j]>s2[j+1])
{
char p=s2[j];
s2[j]=s2[j+1];
s2[j+1]=p;
}
}
}
//printf("%s\n",s2);

if(strcmp(s1,s2)==0)
printf("anagram strings\n");
else
printf("Not an anagram\n");
}*/
