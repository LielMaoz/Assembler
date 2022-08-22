/*
 * data.h
 * 		module handles storing of the data segment and the population of the symbol table
 */
#ifndef DATA_H
#define DATA_H
#include "constraints.h"

typedef struct Symbol{
	char name[MAX_LINE_LENGTH];
	int address;
	int isExternal;
	int segment;
	struct Symbol* next;
} Symbol;

enum SymbolExternalStatus {
							REGULAR_LABEL_SYM,
							EXTERNAL_SYM,
							ENTRY_SYM,
							ENTRY_AWAITING_ADDRESS_SYM
};

enum symbolSegments {
					COMMAND_SEGMENT,
					DATA_SEGMENT
};

/*
 *  Tries to discern a label in the line, Storing it in returnLabel.
 *  Returns the first index after the label (beyond ':') or 0 if not found one
 */
int getLabel (char* line, char *returnLabel);

/*
 * Checks if a label name is valid
 * returns 1 for true 0 for false
 */
int isValidLabelName (char* labelName);

/*
 * Updating the label information and adds it to a linked list of labels 
 * returns 1 for success 0 if label is previously defined
 */
int storeLabel(Symbol** head, char* labelName, int address, int externalStatus, int segment, int lineNumber);

/*
 * Adds the numbers in the line to the data array
 */
int storeDataType(char *line, int* dataArray, int* dc, int lineNumber);

/*
 * Adds the ascii values of the characters in the string to the data array
 */
int storeStringType(char *line, int* dataArray, int* dc, int lineNumber);

/*
 * Adds the number in the line and the ascii values of the characters in the string to the data array
 */
int storeStructType(char *line, int* dataArray, int* dc, int lineNumber);

/*
 * Searching for the symbol name in the symbol table 
 * returns the symbol pointer if found, otherwise returns NULL
 */
Symbol* findSymbolInTable (char* name, Symbol* symTable);

void destroySymbols(Symbol* head);

#endif /* DATA_H */
