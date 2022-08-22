/*
 * Command.h
 *	module handles the decoding of command statements
 */

#ifndef COMMAND_H
#define COMMAND_H

typedef struct CompiledLine {
	char *binaryStr; /*a compiled string of a binary word - or a symbol to be decoded in the second pass*/
	int address; /*address for the word in the memory */
	struct CompiledLine *next; /*pointer to next word */
}CompiledLine;

/*
 * Receives a line containing a command and initial IC and returns a compiled lined list of binaries 
 * or as of yet unrecognizable labels
 * Returns NULL if encountered errors
 */
CompiledLine* decodeCommandLine (char* line, int* ic, int lineNumber);

/*
 * Frees space dynamically allocated to CompiledLine instances
 */
void destroyDecoded(CompiledLine* decoded);

#endif /* COMPILER_H_ */
