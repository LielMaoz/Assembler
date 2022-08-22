/*
 * utilities.c
 * 		contains various helper methods to handle strings
 */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "utilities.h"
#include "constraints.h"

#define TRUE 1
#define FALSE 0
/*
 * Removes leading and trailing whitespace from a string
 * Additionally removes duplicate concurrent whitespaces leaving only one occurrence of ' '
 */
void strTrim(char *str){
    char* ptr = str;
    char* lastNonWhite = ptr;
    int encounteredSpace = FALSE, beginning = TRUE, insideBrackets = FALSE;

    ptr = str;
    while(*str){
    	if(*str=='\"'){
    		insideBrackets = !insideBrackets;
    		beginning = FALSE;
    		*ptr++ = *str;
    		lastNonWhite = ptr;
    		encounteredSpace = FALSE;
    	}
    	else if(isspace(*str)){
            if((!beginning && str[1]!=':' && !encounteredSpace) || insideBrackets){
            	*ptr++ = ' ';
                encounteredSpace = TRUE;
            }
        }
        else{
            beginning = FALSE;
            *ptr++ = *str;
            lastNonWhite = ptr;
            encounteredSpace = FALSE;
        }
        str++;
    }
    *lastNonWhite = '\0';
}

/*
 * Receives a filename and extension and returns a new string containing the two concatenated to a full filename
 */
char* constructUrl(char* name, char* extension){
	char* str = (char*)malloc(MAX_FILE_NAME_LENGTH*sizeof(char));
	strcpy(str,name);
	strcat(str,".");
	strcat(str,extension);
	return str;
}

/*
 * Receives a string of ascii characters representing a number and a pointer to an integer
 * tries to convert the string to a number and store it in the integer
 * Using this and not standard stdlib atoi because standard library does not flag errors
 * returns 1 if succeeded or 0 if encountered errors
 */
int customAtoi (char* num, int* convertedNumber){
    int sign = 1;
    *convertedNumber=0;
    while(isspace(*num))
        num++;
    if(*num =='-'){
        sign = -1;
        num++;
    }
	else if(*num=='+')
		num++;
    while(*num){
        if(!isdigit(*num)){
            *convertedNumber=0;
            return 0;
        }
        *convertedNumber *= 10;
        *convertedNumber += (*num-'0');
        num++;
    }
    *convertedNumber *= sign;
    return 1;
}

/* converts an integer to a sting representation*/
char* customItoa (int value){
    char* numStr = (char*)malloc(sizeof(char)*4);
    int index;
    numStr[3]='\0';
    for(index=2; index>=0; index--){
        numStr[index] = '0'+ (value%10);
        value/=10;
    }
    return numStr;
}
