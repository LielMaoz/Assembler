/*
 * assembly.c
 * 		module provides a function to compile a given assembler code file into object, entry and extern files
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "assembly.h"
#include "utilities.h"
#include "constraints.h"
#include "command.h"
#include "output.h"
#include "data.h"

#define DATA_LENGTH 6
#define STRING_LENGTH 8
#define STRUCT_LENGTH 8
#define ENTRY_LENGTH 7
#define EXTERN_LENGTH 8

enum statementType { emptyStatement, commandStatement, dataStatement, stringStatement, structStatement, entryStatement, externStatement };
enum externalStatus { regularLabel, external, entry };
enum ARE {LOCAL_ARE, EXTERNAL_ARE, RELOCATABLE_ARE };

int firstPass (char* url, char memory[TARGET_MACHINE_MEMORY_LENGTH][MAX_LINE_LENGTH], int* ic, int* dc, Symbol** symTable, int* dataArray);
int prepareSecondPass (Symbol* symTable, int ic);
int secondPass (char* name, char memory[TARGET_MACHINE_MEMORY_LENGTH][MAX_LINE_LENGTH], int ic, int dc, Symbol* symTable, int* dataArray);
int getStatementType (char* line);

/*
 * Manages the assembly process.
 * Receives a .am file name (without extension), performs validation and compiling on the file
 * If file is valid the function writes the compiled object, entry and extern files
 * Returns 1 if succeeded or 0 if encountered errors
 */
int assemble(char *filename){
	char memory [TARGET_MACHINE_MEMORY_LENGTH][MAX_LINE_LENGTH];
	char* inputUrl = constructUrl(filename,"am");
	Symbol *symbolTable = NULL;
	int dc = 0, ic = PROGRAM_LOAD_ADDRESS;
	int success=1;
	int dataArray [TARGET_MACHINE_MEMORY_LENGTH];

	success = firstPass(inputUrl, memory, &ic, &dc, &symbolTable, dataArray);
	if(!success){
		destroySymbols(symbolTable);
		return success;
	}
	free(inputUrl);

	success = prepareSecondPass(symbolTable, ic);
	if(!success){
		destroySymbols(symbolTable);
		return success;
	}

	success = secondPass(filename, memory, ic, dc, symbolTable, dataArray);
	destroySymbols(symbolTable);
	return success;
}

/*
 * Function reads the given filename and decodes what it can while advancing the ic,dc indexes
 * going over the the file it populates the memory, symTable and dataArray,
 * returns 0 if encountered an error otherwise returns 1
 */
int firstPass (char* url, char memory[TARGET_MACHINE_MEMORY_LENGTH][MAX_LINE_LENGTH], int* ic, int* dc, Symbol** symTable, int* dataArray){
	int success=1;
	int i, labelDetectedFlag, lineType, lineNumber = 0;
	FILE* amFile; /*stream for the .am file*/
	CompiledLine *decoded; /*stores the list of decoded words derived from a command */
	CompiledLine *compiledPtr; /*a pointer to traverse the list while retain a reference to the head for freeing it)*/
	char potentialLabel[MAX_LABEL_NAME_LENGTH]; /*if the line has a label it will be stored here*/
	char *buffer = (char*)malloc(MAX_LINE_LENGTH * sizeof(char)); /*used as a line buffer*/
	char *running; /*used to advance within the buffer (retaining a pointer to head for freeing allocated space)*/

	amFile = fopen(url, "r"); /*opening the .am file*/
	if(amFile == NULL){
		fprintf(stderr,"Error: couldn't open file %s\n", url);
		free(buffer);
		return 0;
	}

	while(fgets(buffer, MAX_LINE_LENGTH, amFile)){
		lineNumber++;
		labelDetectedFlag = 0;
		memset(potentialLabel,'\0',MAX_LABEL_NAME_LENGTH);

		strTrim(buffer);

		/* if an empty or comment line then skip over it*/
		if(*buffer == '\0' || *buffer == ';'){
			continue;
		}

		/* if the line starts with a label then retrieve it into potentialLabel*/
		i = getLabel(buffer, potentialLabel);
		labelDetectedFlag = (i==0)? 0:1;

		if(labelDetectedFlag == 1){
			if(!isValidLabelName(potentialLabel)){
				fprintf(stderr,"Error detected in line [%d]: '%s' is not a valid label name\n",lineNumber,potentialLabel);
				success = 0;
				continue;
			}
		}
		running = buffer+i; /*advance line buffer beyond potential label */


		lineType = getStatementType(running);
		switch(lineType){
			case entryStatement:{
				if(labelDetectedFlag){
					printf("Warning: in line [%d], ignored label '%s' before .entry statement\n",lineNumber,potentialLabel);
				}
				running += ENTRY_LENGTH; /*skipping the word '.entry ' */
				if(!isValidLabelName(running)){
					fprintf(stderr,"Error detected in line [%d]: '%s' is not a valid label name \n",lineNumber,running);
					success = 0;
					continue;
				}
				success = storeLabel(symTable, running, lineNumber, ENTRY_SYM, COMMAND_SEGMENT, lineNumber);
			}
				break;
			case externStatement:{
				if(labelDetectedFlag){
					printf("Warning: in line [%d], ignored label '%s' before .extern statement\n",lineNumber,potentialLabel);
				}
				running += EXTERN_LENGTH; /*skipping the word '.extern ' */
				if(!isValidLabelName(running)){
					fprintf(stderr,"Error detected in line [%d]: '%s' is not a valid label name \n",lineNumber,running);
					success = 0;
					continue;
				}
				success = storeLabel(symTable, running, 0, EXTERNAL_SYM, COMMAND_SEGMENT, lineNumber);
			}
				break;
			case emptyStatement: {
				if(labelDetectedFlag){
					success = storeLabel(symTable, potentialLabel, *ic, REGULAR_LABEL_SYM, COMMAND_SEGMENT, lineNumber);
				}
			}
				break;
			case commandStatement:{
				if(labelDetectedFlag){
					success = storeLabel(symTable, potentialLabel, *ic, REGULAR_LABEL_SYM, COMMAND_SEGMENT, lineNumber);
				}
				/* decoded will be a linked list of 1-5 words or NULL if encountered errors*/
				decoded = decodeCommandLine(running, ic, lineNumber);
				compiledPtr = decoded;
				while(compiledPtr){
					strcpy(memory[compiledPtr->address],compiledPtr->binaryStr);
					compiledPtr = compiledPtr->next;
				}

				if(decoded)
					destroyDecoded(decoded);
				else
					success = 0;
			}
				break;
			case dataStatement:{
				if(labelDetectedFlag){
					success = storeLabel(symTable, potentialLabel, *dc, REGULAR_LABEL_SYM, DATA_SEGMENT, lineNumber);
				}
				success = storeDataType(running + DATA_LENGTH, dataArray, dc, lineNumber);
			}
				break;
			case stringStatement:{
				if(labelDetectedFlag){
					success = storeLabel(symTable, potentialLabel, *dc, REGULAR_LABEL_SYM, DATA_SEGMENT, lineNumber);
				}
				success = storeStringType(running + STRING_LENGTH, dataArray, dc, lineNumber);
			}				
				break;
			case structStatement:{
				if(labelDetectedFlag){
					success = storeLabel(symTable, potentialLabel, *dc, REGULAR_LABEL_SYM, DATA_SEGMENT, lineNumber);
				}
				success = storeStructType(running + STRUCT_LENGTH, dataArray, dc, lineNumber);
			}				
				break;
		}
	}
	free(buffer);
	fclose(amFile);
	return success;
}

/*
 * In between first and second pass - advances data segment by ic
 * Also flags error if a .entry symbol was declared but never defined - if so returns 0 otherwise 1
 */
int prepareSecondPass (Symbol* symTable, int ic){
	/*before second pass advance each data segment symbol's address by ic*/
	while(symTable){
		if(symTable->segment == DATA_SEGMENT){
			symTable->address += ic;
		}
		if(symTable->isExternal == ENTRY_AWAITING_ADDRESS_SYM){
			fprintf(stderr,"Error detected in line [%d]: label '%s' is declared as entry but never defined\n",symTable->address, symTable->name);
			return 0;
		}
		symTable= symTable->next;
	}
	return 1;
}

/*
 * Function goes over the memory array, replacing labels with their address
 * Writes compiled words to .ob file
 * Writes extern line numbers to .ext file (if found any)
 * Writes entry symbols and addresses to .ent fil (if found any)
 * returns 0 if encountered errors or 1 if not
 */
int secondPass (char* name, char memory[TARGET_MACHINE_MEMORY_LENGTH][MAX_LINE_LENGTH], int ic, int dc, Symbol* symTable, int* dataArray){
	int success = 1;
	int i, address = PROGRAM_LOAD_ADDRESS, entryDetected=0, externDetected=0;
	char *outputLine, *binary, *label, *lineNumber;
	char copy [MAX_LINE_LENGTH];
	Symbol* symbol;
	char *obUrl, *entUrl, *extUrl;	/* url for output files*/
	FILE *obFile, *entFile, *extFile; /* stream for output files*/

	obUrl = constructUrl(name,"ob");
	entUrl = constructUrl (name, "ent");
	extUrl = constructUrl (name, "ext");

	obFile = fopen(obUrl, "w");
	entFile = fopen(entUrl, "w");
	extFile = fopen(extUrl, "w");

	outputLine = constructObjectFileFirstLine((ic-PROGRAM_LOAD_ADDRESS),dc);
	fprintf(obFile,"%s\n",outputLine);
	free(outputLine);

	for(; address < ic ; address++){
		if(isalpha(memory[address][0])){
			/*first handles labels which were not compiled in the first pass*/
			strcpy(copy,memory[address]);
			label = strtok(copy,"|");
			lineNumber = strtok(NULL,"|");
			symbol = findSymbolInTable(label, symTable);
			if(!symbol){
				fprintf(stderr,"Error in line [%s]: unknown label '%s'\n",lineNumber,label);
				success=0;
				continue;
			}

			else if(symbol->isExternal == EXTERNAL_SYM){
				externDetected=1;
				binary = constructType2Binary(0,EXTERNAL_ARE);
				if(success){
					outputLine = constructEntExtFileLine(symbol->name,address);
					fprintf(extFile,"%s\n",outputLine);
					free(outputLine);
				}
			}
			else{
				binary = constructType2Binary(symbol->address, RELOCATABLE_ARE);
			}
			strcpy(memory[address],binary);
			free(binary);
		}
		if(success){
			outputLine = constructObjectFileLine(address,memory[address]);
			fprintf(obFile,"%s\n",outputLine);
			free(outputLine);
		}
	}
	ic += dc;
	for(i=0; i < dc; i++, address++){
		if(success){
			binary = constructType3Binary (dataArray[i]);
			outputLine = constructObjectFileLine(address, binary);
			fprintf(obFile,"%s\n",outputLine);
			free(outputLine);
		}
	}
	if(success){
		while(symTable){
			if(symTable->isExternal == ENTRY_SYM){
				entryDetected = 1;
				outputLine = constructEntExtFileLine(symTable->name, symTable->address);
				fprintf(entFile,"%s\n",outputLine);
				free(outputLine);
			}
			symTable = symTable->next;
		}
	}


	if(!success){
		remove(obUrl);
		remove(entUrl);
		remove(extUrl);
	}
	else {
		if(!entryDetected){
			printf("No entry directives found, not creating %s file\n",entUrl);
			remove(entUrl);
		}
		if(!externDetected){
			printf("No extern labels found, not creating %s file\n",extUrl);
			remove(extUrl);
		}
	}
	free(obUrl);
	free(extUrl);
	free(entUrl);
	fclose(obFile);
	fclose(entFile);
	fclose(extFile);

	return success;
}

/*
 * Receives a line (without a label declaration) and returns the type as an integer (using enum statementType)
 */
int getStatementType (char* line){
	char word [MAX_TYPE_LENGTH];
	int index;
	strTrim(line);
	if(strcmp(line,"\0")==0)
		return emptyStatement;
	for(index = 0; index < MAX_TYPE_LENGTH-1 && *line && !isspace(*line) ;line++, index++){
		word[index] = *line;
	}
	word[index] = '\0';
	if(strcmp(word,".entry")==0) return entryStatement;
	if(strcmp(word,".extern")==0) return externStatement;
	if(strcmp(word,".data")==0) return dataStatement;
	if(strcmp(word,".string")==0) return stringStatement;
	if(strcmp(word,".struct")==0) return structStatement;
	return commandStatement;
}
