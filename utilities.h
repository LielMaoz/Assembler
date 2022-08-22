/*
 * utilities.h
 * 		contains various helper methods to handle strings
 */
#ifndef STRING_UTILITIES_H_
#define STRING_UTILITIES_H_

/*
 * Removes leading and trailing whitespace from a string
 * Additionally removes duplicate concurrent whitespaces leaving only one occurrence of ' '
 */
void strTrim(char *str);

/*
 * Receives a filename and extension and returns a new string containing the two concatenated to a full filename
 */
char* constructUrl(char* name, char* extension);

/*
 * Receives a string of ascii characters representing a number and a pointer to an integer
 * tries to convert the string to a number and store it in the integer
 * Using this and not standard stdlib atoi because standard library does not flag errors
 * returns 1 if succeeded or 0 if encountered errors
 */
int customAtoi (char* num, int* convertedNumber);

/* converts an integer to a sting representation*/
char* customItoa (int value);

#endif
