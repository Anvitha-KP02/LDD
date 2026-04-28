/*Problem Statement
------------------------------------------------------------------------------------
Given a string s consisting of words and spaces, return the length of the last word in the string.
A word is a maximal substring consisting of non-space characters only.

Example 1:
Input: s = "Hello World"
Output: 5

Example 2:
Input: s = "   fly me   to   the moon  "
Output: 4

Example 3:
Input: s = "luffy is still joyboy"
Output: 6
-----------------------------------------------------------------------------------
*/

#include<stdio.h>
#include<string.h>
void main()
{
	char s[100];
	int i,len,c;
	printf("Enter the string:\n");
	scanf("%[^\n]",s);
	len=strlen(s);
	for(i=len-1;i>=0 && s[i]==' '; i--);
	for(c=0;i>=0 && s[i]!=' ';i--)
		c++;
	printf("Count=%d\n",c);
}
