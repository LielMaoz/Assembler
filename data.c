/*
 * data.c
 * 		module handles storing of the data segment and the population of the symbol table
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "data.h"
#include "constraints.h"
#include "utilities.h"

#define NUM_OF_RESERVED_WORDS 24
#define MAX_INSTRUCTION 8
#define NUM_OF_INSTRUCTIONS_TYPE 16
#define NUM_OF_DATA_TYPE 16
#define MAX_CHAR_IN_BASE 10

/*
 *  Tries to discern a label in the line, Storing it in returnLabel.
 *  Returns the first index after the label (beyond ':') or 0 if not found one
 */
int getLabel (char* line, char *returnLabel){
	int index = 0;
	/* initial check to see if the line contains a ':' sign at all - if not then there's no label*/
	if(!strchr(line,':')){
		returnLabel[index]='\0';
		return index;
	}
	/*
	 * knowing the line contains a ':' symbol, trying to isolate a label
	 * making sure the symbol ':' isn't part of a string (e.g. ".string 'a:a' " is a valid statement) 
	 */ 
	for(; index < MAX_LABEL_NAME_LENGTH && *line && !isspace(*line) && *line!='.'; line++, index++){
		if(*line != ':')
			returnLabel[index] = *line; /* add character to potential name buffer */
		else{
			returnLabel[index] = '\0'; /* once detected cap buffer and return index*/
			return index+1;
		}
	}
	/*if not found label */
	index = 0;
	returnLabel[index]='\0';
	return index;
}

/*
 * Checks if a label name is valid
 * returns 1 for true 0 for false
 */
int isValidLabelName (char* labelName){
	char *reservedWords[] = {
			"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc",
			"dec", "jmp", "bne", "get", "prn", "jsr", "rts", "hlt",
			"r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6", "r7"
	};
	int j = 0;

	/* checking if label name isn't NULL */
	if(!labelName){
		return 0;
	}

	/* checking if label name isn't a reserved words */
	for(; j < NUM_OF_RESERVED_WORDS ; j++){
		if(strcmp(labelName, *(reservedWords + j)) == 0)
			return 0;
	}

	/* checking first character is a letter */
	if(!isalpha(*labelName)){
		return 0;
	}

	/* checking each subsequent character is either letter or number */
	while(*labelName){
		if(!isalnum(*labelName)){
			return 0;
		}
		labelName++;
	}
	return 1;
}

int storeLabel(Symbol** head, char* labelName, int address, int externalStatus, int segment, int lineNumber){
	Symbol *current = findSymbolInTable(labelName,*head);

	if(current){
		if(current->isExternal == ENTRY_AWAITING_ADDRESS_SYM && externalStatus == REGULAR_LABEL_SYM){
			current->isExternal = ENTRY_SYM;
			current-> address = address;
			current-> segment = segment;
			return 1;
		}
		if(current->isExternal == REGULAR_LABEL_SYM && externalStatus == ENTRY_SYM){
			current->isExternal = ENTRY_SYM;
			return 1;
		}
		fprintf(stderr,"Error detected in line [%d]: '%s' was previously defined.\n",lineNumber, labelName);
		return 0;
	}
	current = (Symbol*)malloc(sizeof(Symbol));
	memset(current->name, '\0', MAX_LABEL_NAME_LENGTH);
	strcpy(current->name, labelName);
	current-> address = address;
	current-> isExternal = (externalStatus==ENTRY_SYM)? ENTRY_AWAITING_ADDRESS_SYM : externalStatus;
	current-> segment = segment;
	current-> next = *head;
	*head = current;
	return 1;
}

int storeDataType(char *line, int* dataArray, int* dc, int lineNumber){
	int num, flag;
	char comma[2] = ",";
	char *decimalNumber;
	int* startDc = dc;
	if(!(*line)){
		fprintf(stderr, "Error detected in line [%d]: missing numbers after .data\n", lineNumber);
		return 0;
	}
	if(*line == ','){
		fprintf(stderr, "Error detected in line [%d]: illegal comma\n", lineNumber);
		return 0;
	}
	decimalNumber = strtok(line, comma);
	flag = customAtoi(decimalNumber, &num);
	if(!flag){
		fprintf(stderr, "Error detected in line [%d]: %s is not a number\n", lineNumber, decimalNumber);
		return 0;
	}
	dataArray[(*dc)++] = num;
	for(; (decimalNumber = strtok(NULL, comma)) ;(*dc)++){
		flag = customAtoi(decimalNumber, &num);

		if(!flag){
			fprintf(stderr, "Error detected in line [%d]: %s is not a number\n", lineNumber, decimalNumber);
			dc = startDc;
			return 0;
		}
		dataArray[*dc] = num;
	}

	return 1;
}

int storeStringType(char *line, int* dataArray, int* dc, int lineNumber){
	int num;
	int* startDc = dc;
	for( ; isspace(*line) ; line++){
	} /* skips tabs and spaces at the beginning of the line */	
	
	if(*line == '\"'){
		for(line++; *line && *line != '\"' ;(*dc)++, line++){
			num = *line;
			if(!isalpha(*line)){
				fprintf(stderr, "Error detected in line [%d]: %c is not a character\n", lineNumber, *line);
				return 0;
			}
			dataArray[*dc] = num;
		}
		if(*line == '\"'){
			dataArray[(*dc)++] = 0;
			return 1;
		}
		else{
			fprintf(stderr, "Error detected in line [%d]: missing string\n", lineNumber);
			dc = startDc;
			return 0;
		}
	}
	fprintf(stderr, "Error detected in line [%d]: missing string\n", lineNumber);
	return 0;
}

int storeStructType(char *line, int* dataArray, int* dc, int lineNumber){
	int i = 1, num, flag;
	char comma[2] = ",";
	char *token;
	if(!(*line)){
		fprintf(stderr, "Error detected in line [%d]: missing information after .struct\n", lineNumber);
		return 0;
	}
	token = strtok(line, comma);
	flag = customAtoi(token, &num);
	if(!flag){
		fprintf(stderr, "Error detected in line [%d]: %s is not a number\n", lineNumber, token);
		return 0;
	}	
	dataArray[(*dc)++] = num;
	token = strtok(NULL, comma);
	for(i++; token[i] && token[i] != '\"' ; i++){
		num = token[i];
		if(!isalpha(num)){
			fprintf(stderr, "Error detected in line [%d]: %c is not a character\n", lineNumber, token[i]);
			return 0;
		}
		dataArray[(*dc)++] = num;
	}
	dataArray[(*dc)++] = 0;
	return 1;
}

Symbol* findSymbolInTable (char* name, Symbol* symTable){
	Symbol* ptr = symTable;

	while(ptr){
		if(strcmp(ptr->name,name)== 0){
			return ptr;
		}
		ptr = ptr->next;
	}
	return NULL;
}


void destroySymbols(Symbol* head){
	if(head == NULL){
		free(head);
		return;
	}
	destroySymbols(head->next);
	free(head);
}


/* debugging */
void printDataArray(int* dataArray, int ic, int dc){
	int i = 0, code, address = (int)ic, dataCount = (int)dc;
	printf("number of data lines is: %d\n", dc);
	for( ; i < dataCount ; i++, address++){
		code = dataArray[i];
		printf("address is: %d\tcode is: %d\n", address, code);
	}
}



void printSymbols(Symbol* head){
	while(head){
		printf("label name is: %s\taddress is: %d\n\n", head->name, head->address);
		head = head->next;
	}
}
