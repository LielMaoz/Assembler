#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "data.h"

extern int assemble(char *name);
extern void preprocessor(char *name, int *success);

void assembleFile(char *filename);

int main(int argc, char **argv){
	int i=1;
	if (argc < 2)
		printf("No command line parameters found.\nProgram requires files to compile and assemble.\nPlease enter .as file names (without extension) as command line parameters.\n");
	for(; i<argc; i++){
		printf("Begin operation on %s.as\n",argv[i]);
		assembleFile(argv[i]);
		printf("-------------\n");
	}
	return 1;
}

void assembleFile(char *filename){
	int success=1;
	printf("Performing pre processor\n");
	preprocessor(filename, &success);
	if(!success){
		printf("Encountered error during preprocessor - aborting operation\n");
		return;
	}
	printf("Beginning work on expanded file %s.am\n",filename);
	success = assemble(filename);
	if(success)
		printf("Finished Assembly Process on %s successfully\n",filename);
	else
		fprintf(stderr,"Program encountered errors while assembling file %s.am, aborting operation.\n",filename);
}
