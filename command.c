/*
 * Command.c
 *	module handles the decoding of command statements
 */

#include "command.h"
#include "output.h"
#include "utilities.h"
#include "constraints.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/*const declarations */
enum operandType { IMMEDIATE_OP,  LABEL_OP, STRUCT_OP, REGISTER_OP}; /* type of operands - immediate, label, struct, register*/

static char *validCommands[] = {
		"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc",
		"dec", "jmp", "bne", "get", "prn", "jsr", "rts", "hlt"
};

/*static char *registers[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};*/
static int legalAmountOperands[16] = {2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 };

/* 
 * details which operator types are legal for which command
 * immediate src, label src, struct src, register src
 * immediate dst, label dst, struct dst, register dst
 */
static int legalOperandTypes [8][16] = {
		{1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 }
};

static char operandTypes [4][10]= { "Immediate", "Label", "Struct", "Register" };

typedef struct Operand {
	char op [30];
	int type; /* enum operator types*/
	int numField;
	/*numField :
	 * if type=reg then register number
	 * if type=immediate then value
	 * if struct then index and op will only contain the label name
	 * if label then it remains undefined
	 * */
} Operand;

typedef struct Command {
	int opCode;
	Operand* srcOp;
	Operand* dstOp;
} Command;

typedef struct CrudeCommand {
	char* command;
	char* op1;
	char* op2;
} CrudeCommand;

/* private functions declaration */

CrudeCommand* initialDeconstruction (char* line, int lineNumber);
Command* validateCrudeCommand (CrudeCommand* crud, int lineNumber);
CompiledLine* finalEncoding (Command* cmd, int* ic, int lineNumber);

CompiledLine* decodeOperand (Operand* op, int isSrc, int *ic, int lineNumber);
CompiledLine* decodeLabel (Operand* op, int *ic, int lineNumber);
CompiledLine* decodeImmediate (Operand* op, int* ic);
CompiledLine* decodeRegister (Operand* op1, Operand* op2, int* ic);

void destroyCrude (CrudeCommand* c);
void destroyCommand (Command* cmd);

int isLegalCommand (Command c, int lineNumber);
Operand* constructOperand (char* op, int lineNumber);

/*decode */
/*
 * Receives a line containing a command and initial IC and returns a compiled lined list of binaries 
 * or as of yet unrecognizable labels
 * Returns NULL if encountered errors
 */
CompiledLine* decodeCommandLine (char* line, int* ic, int lineNumber){
	CrudeCommand* crud; /*stores up to 3 strings - command, operand1, operand 2 */
	Command* cmd;	/*stores command op code, and for each operand strores type and relavant descernable info*/
	CompiledLine* ret = NULL; /*stores either the compiled 10 character binary word (as string) or a label name*/

	crud = initialDeconstruction (line, lineNumber);
	cmd = validateCrudeCommand (crud, lineNumber);
	ret = finalEncoding (cmd, ic, lineNumber);
	
	destroyCrude(crud);
	destroyCommand(cmd);
	return ret;
}

/*
 * Phase I of decoding - using strtok breaks down the string to up to three strings:
 * 			command and two optional operators
 *
 */
CrudeCommand* initialDeconstruction (char* line, int lineNumber){
	CrudeCommand* cmd = (CrudeCommand*)malloc(sizeof(CrudeCommand));
	char cpy [MAX_LINE_LENGTH]; /*created copy because strtok destroys origninal */
	char *t;

	cmd->command = NULL;
	cmd->op1 = NULL;
	cmd->op2 = NULL;

	strcpy(cpy,line);
	t = strtok(cpy," ");
	if(t){
		cmd->command = (char*)malloc(sizeof(char)*30);
		strcpy(cmd->command,t);
	}
	t = strtok(NULL,",");
	if(t!=NULL){
		cmd->op1 = (char*)malloc(sizeof(char)*30);
		strcpy(cmd->op1,t);
	}
	else{
		return cmd;
	}
	t = strtok(NULL,",");
	if(t!=NULL){
		cmd->op2 = (char*)malloc(sizeof(char)*30);
		strcpy(cmd->op2,t);
	}
	t = strtok(NULL,",");
	if(t!=NULL){
		fprintf(stderr, "Error detected in line [%d]: illegal amount of operands (more than 2)\n",lineNumber);
		destroyCrude(cmd);
		return NULL;
	}

	return cmd;
}

/*
 * Phase II of decoding - analyzes the string components and checks if valid command
 */
Command* validateCrudeCommand (CrudeCommand* crud, int lineNumber){
	Command* cmd;
	int i,flag;
	if(!crud){
		return NULL;
	}
	cmd = (Command*)malloc(sizeof(Command));

	for(i=0; i<16; i++){
		flag = strcmp(crud->command, validCommands[i]);
		if(flag==0){
			cmd->opCode = i;
			i=16;
		}
	}
	if(flag){
		fprintf(stderr, "Error detected in line [%d]: unrecognized command '%s'\n",lineNumber, crud->command);
		free(cmd);
		return NULL;
	}


	if(!crud->op1){
		/*no op1 => no op2*/
		cmd->srcOp = NULL;
		cmd->dstOp = NULL;
	}
	else if(!crud->op2){
		/*^every command that accepts only one operand accepts it as dst*/
		cmd->srcOp = NULL;
		cmd->dstOp = constructOperand(crud->op1, lineNumber);
		if(!cmd->dstOp){
			free(cmd);
			return NULL;
		}
	}
	else{
		cmd->srcOp = constructOperand(crud->op1, lineNumber);
		if(!cmd->srcOp){
			free(cmd);
			return NULL;
		}
		cmd->dstOp = constructOperand(crud->op2, lineNumber);
		if(!cmd->dstOp){
			free(cmd);
			return NULL;
		}
	}

	if(!isLegalCommand(*cmd, lineNumber)) {
		free(cmd);
		return NULL;
	}
	return cmd;
}

/*
 * Phase III of decoding - given a valid command returns linked list of compiled binaries or yet unrecognizable labels
 */
CompiledLine* finalEncoding (Command* cmd, int* ic, int lineNumber){
	CompiledLine *retH = NULL;
	CompiledLine *newCompiledLine,*ptr;
	int tO1=0;
	int tO2=0;

	if(!cmd)
		return NULL;


	if (cmd->srcOp){
		tO1 = cmd->srcOp->type;
		strTrim(cmd->srcOp->op);
	}
	if (cmd->dstOp){
			tO2 = cmd->dstOp->type;
			strTrim(cmd->dstOp->op);
	}

	newCompiledLine = (CompiledLine*)malloc(sizeof(CompiledLine));
	newCompiledLine->binaryStr = constructType1Binary(cmd->opCode,tO1,tO2,0);
	newCompiledLine->address = (*ic)++;
	newCompiledLine->next = retH;
	retH = newCompiledLine;


	/*
	 * if both registers are used it's handled together
	 *		otherwise each operand is handled separately
	 */
	if(tO1 == tO2 && tO1==REGISTER_OP){
		newCompiledLine = decodeRegister(cmd->srcOp,cmd->dstOp,ic);
		newCompiledLine->next = retH;
		retH = newCompiledLine;
		return retH;
	}

	newCompiledLine = decodeOperand(cmd->srcOp, 1, ic, lineNumber);
	if(newCompiledLine){
		ptr = newCompiledLine;
		while(ptr->next){
			ptr = ptr->next;
		}
		ptr->next = retH;
		retH = newCompiledLine;
	}

	newCompiledLine = decodeOperand(cmd->dstOp, 0, ic, lineNumber);
	if(newCompiledLine){
		ptr = newCompiledLine;
		while(ptr->next){
			ptr = ptr->next;
		}
		ptr->next = retH;
		retH = newCompiledLine;
	}

	return retH;
}



int isLegalCommand (Command c, int lineNumber){
	int amountReg=0;
	if(c.srcOp){
		amountReg++;
		if(!(legalOperandTypes[c.srcOp->type][c.opCode])){
			fprintf(stderr, "Error detected in line [%d]: incompatible source operand of type '%s' for the command '%s'\n",lineNumber,operandTypes[c.srcOp->type],validCommands[c.opCode]);
			return 0;
		}
		if(c.srcOp->type == IMMEDIATE_OP){
			if(c.srcOp->numField>127 || c.srcOp->numField<-127){
				fprintf(stderr, "Error detected in line [%d]: immediate value exceeds bounds of [-127,127]\n",lineNumber);
				return 0;
			}
		}
		if(c.srcOp->type == STRUCT_OP && (c.srcOp->numField>2 || c.srcOp->numField<1)){
			fprintf(stderr, "Error detected in line [%d]: struct directives can only access 1st or 2nd field\n",lineNumber);
				return 0;
		}
	}
	if(c.dstOp){
		amountReg++;
		if(!(legalOperandTypes[4+c.dstOp->type][c.opCode])){
			fprintf(stderr, "Error detected in line [%d]: incompatible destination operand of type '%s' for the command' %s'\n",lineNumber,operandTypes[c.dstOp->type],validCommands[c.opCode]);
			return 0;
		}
		if(c.dstOp->type == IMMEDIATE_OP){
			if(c.dstOp->numField>127 || c.dstOp->numField<-127){
				fprintf(stderr, "Error detected in line [%d]: immediate value exceeds bounds of [-127,127]\n",lineNumber);
				return 0;
			}
		}
		if(c.dstOp->type == STRUCT_OP && (c.dstOp->numField>2 || c.dstOp->numField<1)){
			fprintf(stderr, "Error detected in line [%d]: struct directives can only access 1st or 2nd field\n",lineNumber);
				return 0;
		}
	}
	if(legalAmountOperands[c.opCode]!=amountReg){
		fprintf(stderr, "Error detected in line [%d]: illegal amount of operands for the command '%s'\n",lineNumber, validCommands[c.opCode]);
		return 0;
	}
	return 1;
}

/*
 * Given the string containing the operand construct an instance of Operand
 */
Operand* constructOperand (char* op, int lineNumber){
	Operand* o = (Operand*)malloc(sizeof(Operand));
	int num;
	char* token;
	strTrim(op);
	if(op[0]=='#'){
		o->type = IMMEDIATE_OP;
		op++;
		if(customAtoi(op,&num)){
			o->numField = num;
		}
		else{
			/*invalid immediate*/
			printf("Error detected in line [%d]: invalid number for immediate value\n",lineNumber);
			free(o);
			return NULL;
		}
		
		return o;
	}
	if(op[0]=='r'){
		op++;
		if(customAtoi(op,&num) && num<8 && num>-1){
			o->type = REGISTER_OP;
			o->numField = atoi(op);
			return o;
		}
		else
			op--;
	}
	if(strchr(op,'.')){
		token = strtok(op,".");
		strcpy(o->op,token);
		o->type = STRUCT_OP;
		token=strtok(NULL,".");
		o->numField = atoi(token);
		return o;
	}
	strcpy(o->op,op);
	o->type = LABEL_OP;
	return o;
}


/*
 * Handles the decoding of an operand and directs it to each usecase based on type
 */
CompiledLine* decodeOperand (Operand* op, int isSrc, int *ic, int lineNumber){
	CompiledLine* comp;

	if(!op)
		return NULL;
	switch(op->type){
		case IMMEDIATE_OP: comp = decodeImmediate(op,ic);
			break;
		case LABEL_OP: comp = decodeLabel(op,ic, lineNumber);
			break;
		case STRUCT_OP:{
			comp = decodeLabel(op,ic, lineNumber);
			comp->next = decodeImmediate(op,ic);
		}
			break;
		case REGISTER_OP:{
			if(isSrc) comp = decodeRegister(op, NULL, ic);
			else comp = decodeRegister(NULL, op, ic);
		}
			break;
	}
	return comp;
}

/*
 * Constructs the CompiledLine from an Operand of type label
 */
CompiledLine* decodeLabel (Operand* op, int *ic, int lineNumber){
	CompiledLine *comp = (CompiledLine*)malloc(sizeof(CompiledLine));
	char* lineNumberString = customItoa(lineNumber);

	comp -> binaryStr = (char*)malloc(sizeof(char)*MAX_LABEL_NAME_LENGTH);
	strcpy(comp-> binaryStr,op->op);
	strcat(comp-> binaryStr, "|");
	strcat(comp-> binaryStr, lineNumberString); /*used in the second pass for printing errors*/
	comp -> address = (*ic)++;
	comp -> next = NULL;
	free(lineNumberString);
	return comp;
}

/*
 * Constructs the CompiledLine from an Operand of type immediate
 */
CompiledLine* decodeImmediate (Operand* op, int* ic){
	CompiledLine* comp = (CompiledLine*)malloc(sizeof(CompiledLine));
	comp -> binaryStr = constructType2Binary(op->numField,0);
	comp -> address = (*ic)++;
	comp->next = NULL;
	return comp;
}

/*
 * Constructs the CompiledLine from Operands of type register
 * handles one or two operands (pass NULL pointer to use only one)
 */
CompiledLine* decodeRegister (Operand* op1, Operand* op2, int* ic){
	int r1,r2;
	CompiledLine* comp;

	r1 = (op1)? op1->numField:0;
	r2 = (op2)? op2->numField:0;

	comp = (CompiledLine*)malloc(sizeof(CompiledLine));
	comp->binaryStr = constructType4Binary(r1,r2);
	comp->address = (*ic)++;
	comp->next = NULL;
	return comp;
}


/* destructor methods */
/*
 * Frees space dynamically allocated to CrudeCommand instances
 */
void destroyCrude (CrudeCommand* c){
	if(!c) return;
	if(c->command) free(c->command);
	if(c->op1) free(c->op1);
	if(c->op2) free(c->op2);
	free(c);
}

/*
 * Frees space dynamically allocated to Command instances
 */
void destroyCommand(Command* cmd){
	if(!cmd)
		return;
	if(cmd->srcOp){
		free(cmd->srcOp);
		cmd->srcOp = NULL;
	}
	if(cmd->dstOp){
		free(cmd->dstOp);
		cmd->dstOp = NULL;
	}
	free(cmd);
}

/*
 * Frees space dynamically allocated to CompiledLine instances
 */
void destroyDecoded(CompiledLine* decoded){
	if(!decoded){
		return;
	}
	destroyDecoded(decoded->next);
	decoded->next = NULL;
	free(decoded);
}
