/*
 * preprocessor.c
 * 		module is in charge of initially reading the .as file and replacing macro statements saving
 * 		the code to .am file
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "utilities.h"
#include "constraints.h"
#include "preprocessor.h"

#define MACRO 6
#define ENDMACRO 8
#define NUM_OF_RESERVED_WORDS 21

typedef struct Macro{
	char name[MAX_LINE_LENGTH];
	char text[TARGET_MACHINE_MEMORY_LENGTH];
	struct Macro* next;
} Macro;

void getMacroName(char* line, char* macroName);
int isValidMacroName (Macro* head, char* macroName);
Macro* newMacro(char* macroName, char* macroContent);
void storeMacro(Macro **head, char* macroName, char* macroContent);
void getMacroContent(char* line, char* macroContent, FILE *f1);
int isMacroOrEndmacro(char* line, char* macroOrEndmacro);
int putMacro(Macro* head, char* line, FILE *f2);
void destroy(Macro* head);

/*
 * Tries to read the file "[name].as", treats macro declarations
 * and saves the completed file as file "[name].am"
 */
void preprocessor(char *name, int *success){
	FILE *asFile, *amFile;
	char *inputUrl, *outputUrl;
	char *buffer = NULL;
	Macro *head = NULL;
	char* isMacro = "macro";
	buffer = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));
	inputUrl = constructUrl(name,"as");
	asFile = fopen(inputUrl, "r");

	if(asFile == NULL){
		fprintf(stderr,"Error: couldn't read file %s!\n\t\tMake sure file name is correct.\n",inputUrl);
		free(buffer);
		free(inputUrl);
		*success = 0;
		return;
	}
	outputUrl = constructUrl(name,"am");
	amFile = fopen(outputUrl, "w");
	if(amFile == NULL){
		fprintf(stderr,"Error: couldn't create file %s!\n\n",outputUrl);
		fclose(asFile);
		remove(outputUrl);
		free(buffer);
		*success =0;
		return;
	}
	while(fgets(buffer, MAX_LINE_LENGTH, asFile)){ /* reading a line from source file */
		if(isMacroOrEndmacro(buffer, isMacro)){ /* if the first word in the line is "macro" and macro name is legal - it stores the macro in a linked list */
			char macroName[MAX_LINE_LENGTH];
			char macroContent[MAX_LINE_LENGTH];
			getMacroName(buffer, macroName);
			if(!isValidMacroName(head, macroName)){
				if(!remove(outputUrl)){
					printf("Error detected: macro name '%s' is not valid. Failed to create an expanded source file from %s.\n", macroName,inputUrl);
					*success = 0;
				}
				break;
			}
			getMacroContent(buffer,macroContent, asFile);
			storeMacro(&head, macroName, macroContent);
		}
 		else if(!putMacro(head, buffer, amFile))
			fputs(buffer, amFile);
	}
	fclose(asFile);
	fclose(amFile);
	destroy(head);
	free(buffer);
	return;
}	


void getMacroName(char* line, char* macroName){
	int i = 0;
	char* findMacro = strstr(line, "macro");
	int j = findMacro - line;
	char* running = line + j + MACRO;
	memset(macroName, '\0', MAX_LINE_LENGTH);
	for(; isspace(*running) ; running++){
	} /* skips tabs and spaces after the word "macro" */
	for(; *running && !isspace(*running) ; running++, i++){
		macroName[i] = *running;
	}
}

/*
 * Checks if a macro name is valid
 * returns 1 for true 0 for false
 */
int isValidMacroName (Macro* head, char* macroName){
	int j = 0;
	char *reservedWords[] = {
			"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc",
			"dec", "jmp", "bne", "get", "prn", "jsr", "rts", "hlt",
			 "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7"
			".data", ".string",".struct", ".entry", ".extern"
	};
	if(!macroName){ /* checking if macro name isn't NULL */
		return 0;
	}
	for(; j < NUM_OF_RESERVED_WORDS ; j++){ /* checks if macro name isn't a reserved words */
		if(!strcmp(macroName, *(reservedWords + j)))
			return 0;
	}
	while(head){ /* checks if there is a macro with the same name */
		if(!strcmp(head->name, macroName)){
			return 0;
		}
		head = head->next;
	}
	if(!isalpha(*macroName)){ /* checking first character is a letter */
		return 0;
	}
	while(*macroName){ /* checking each subsequent character is either letter or number */
		if(!isalnum(*macroName)){
			return 0;
		}
		macroName++;
	}
	return 1;
}

/* 
 * Saves the content of the macro
 */
void getMacroContent(char* line, char* macroContent, FILE *f1){
	char* isEndmacro = "endmacro";
	memset(macroContent, '\0', MAX_LINE_LENGTH);
	fgets(line, MAX_LINE_LENGTH, f1);
	for(; !isMacroOrEndmacro(line, isEndmacro); fgets(line, MAX_LINE_LENGTH, f1)){
		strcat(macroContent, line);
	}
}

/*
 * Creates a new macro
 * returns the macro
 */
Macro* newMacro(char* macroName, char* macroContent){
	Macro* current = NULL;	
	current = (Macro*)malloc(sizeof(Macro));
	if(current != NULL){
		strcpy(current->name, macroName);
		strcpy(current->text, macroContent);
		current->next = NULL;
	}
	return current;
}	
	
/*
 * Adds the macro to a linked list of labels 
 */	
void storeMacro(Macro **head, char* macroName, char* macroContent){
	Macro* current;	
	current = newMacro(macroName, macroContent);
	if(current != NULL){
		current->next = *head; 
		*head = current;
	}
}

/*
 * Checks if the line starts with "macro" or "endmacro"
 * returns 1 for true 0 for false
 */
int isMacroOrEndmacro(char* line, char* macroOrEndmacro){
	int i = 0;
	char word[ENDMACRO + 1];
	char* running = line;
	for( ; isspace(*running) ; running++){
	} /* skips tabs and spaces at the beginning of the line */	
	for(; *running && i < ENDMACRO && !isspace(*running) ; running++, i++){
		word[i] = *running;
	}
	word[i] = '\0';
	if(strcmp(word, macroOrEndmacro)) /* if the strings are not equal */
		return 0;
	return 1; /* if the strings are equal */
}

/*
 * Writes the content of the macro in the expanded source file instead of macro name
 * returns 1 if line starts with a macro name 0 if not
 */
int putMacro(Macro* head, char* line, FILE *f2){ 
	int i = 0;	
	char* running = line;
	char nameBuf[MAX_LINE_LENGTH];
	for( ; isspace(*running) ; running++){
	} /* skips tabs and spaces at the beginning of the line */	
	for(; *running && !isspace(*running) ; running++, i++){
		nameBuf[i] = *running;
	}
	nameBuf[i] = '\0';
	while(head){
		if(!strcmp(head->name, nameBuf)){
			fputs(head->text, f2);
			return 1;
		}
		head = head->next;
	}
	return 0;
}

void destroy(Macro* head){
	if(head == NULL){
		free(head);
		return;
	}
	destroy(head->next);
	free(head);
}
