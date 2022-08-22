/*
 * Output.c
 *	module in charge of constructing the different strings of decoded data
 *	and formatting it for the file output in the special base 32
 *
 */
#include "output.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define BINARY_STRING_LENGTH 11
#define BASE_32_STRING_LENGTH 3

/*
 *  Puts the binary representation of given value starting at a given index
 */
void fillBinaryBits (char* string, int value, int start){
	int bit, sign;
	int s = start,overflow;

	sign= (value>=0)? 1:-1;

	value = abs(value);
	while(value>0 && start>=0){
		bit = value % 2;
		value>>=1;
		string[start--] = (bit+'0');
	}

	/*if value is negative - perform 2's complement on the string*/
	overflow = 1;
	if(sign==-1){
			for(;s>=0;s--){
			string[s] = (string[s]=='0')? '1':'0'; /*performs not on bit*/
			if(overflow){
				string[s] = (string[s]=='0')? '1':'0';
				overflow = (string[s]=='0')? 1:0;
			}
		}
	}
}

/*
 * 	Formats a machine code instruction in binary
 */
char* constructType1Binary (int opCode, int typeOpSrc, int typeOpDst, int are){
	char* out = (char*)malloc(sizeof(char)*BINARY_STRING_LENGTH);
	int ind = 0;

	for(;ind < BINARY_STRING_LENGTH; ind++){
		out[ind]='0';
	}

	fillBinaryBits(out, opCode, 3);
	fillBinaryBits(out, typeOpSrc, 5);
	fillBinaryBits(out, typeOpDst, 7);
	fillBinaryBits(out, are, 9);

	out[BINARY_STRING_LENGTH-1] = '\0';

	return out;
}

/*
 * Formats the added word needed for Immediate values or Labels
 */
char* constructType2Binary (int value, int are){
	char* out = (char*)malloc(sizeof(char)*BINARY_STRING_LENGTH);
	int ind = 0;

	for(;ind < BINARY_STRING_LENGTH; ind++){
		out[ind]='0';
	}

	fillBinaryBits(out, value, 7);
	fillBinaryBits(out, are, 9);

	out[BINARY_STRING_LENGTH-1] = '\0';

	return out;
}

/*
 * Formats the added word needed for data members
 */
char* constructType3Binary (int value){
	char* out = (char*)malloc(sizeof(char)*BINARY_STRING_LENGTH);
	int ind = 0;
	for(;ind < BINARY_STRING_LENGTH; ind++){
		out[ind]='0';
	}

	fillBinaryBits(out, value, 9);
	out[BINARY_STRING_LENGTH-1] = '\0';

	return out;
}

/*
 * Formats the added word needed for register numbers in binary
 *	unused register should pass 0
 */
char* constructType4Binary (int regSrc, int regDst){
	char* out = (char*)malloc(sizeof(char)*BINARY_STRING_LENGTH);
	int ind = 0;
	for(;ind < BINARY_STRING_LENGTH; ind++){
		out[ind]='0';
	}

	fillBinaryBits(out, regSrc, 3);
	fillBinaryBits(out, regDst, 7);

	out[BINARY_STRING_LENGTH-1] = '\0';

	return out;
}


/*
 * Receives a string of a binary sequence and converts the subsequence [start,end] to an Integer
 */
int binaryAtoi (char* binary, int start, int end){
	int i = start, conNum;

	conNum = 0;
	for(;i <= end; i++){
		conNum<<=1;
		conNum +=(binary[i]-'0');
	}

	return conNum;
}

/*
 * Receives a string of a 10 bit binary sequence and returns a converted base 32 string
 */
char* convertBinaryStringtoBase32 (char* binary){
	char* b32 = "!@#$%^&*<>abcdefghijklmnopqrstuv";
	char* outP = (char*)malloc(sizeof(char)*BASE_32_STRING_LENGTH);
	outP[0] = b32[binaryAtoi(binary,0,4)];
	outP[1] = b32[binaryAtoi(binary,5,9)];
	outP[2] = '\0';
	return outP;
}

/*
 * Receives a decimal integer and returns a converted base 32 string
 */
char* convertDecimalBase32(int decimalNum){
	int i = 0, j;
	char temp;
	char *toSpecial = (char*)malloc(sizeof(char)*BASE_32_STRING_LENGTH);
	char *b32 = "!@#$%^&*<>abcdefghijklmnopqrstuv";
	if(!decimalNum){
		toSpecial[i++] = b32[0];
		toSpecial[i] = '\0';
		return toSpecial;
	}
	for(; decimalNum ; i++){
		toSpecial[i] = b32[decimalNum % 32];
		decimalNum /= 32;
	}
	toSpecial[i] = '\0';
	j = i - 1;
	for(i = 0; i < j ; i++){
		temp = toSpecial[j];
		toSpecial[j] = toSpecial[i];
		toSpecial[i] = temp;
	}
	return toSpecial;
}

char* constructObjectFileFirstLine (int ic, int dc){
	char* out = (char*)malloc(sizeof(char)*5);
	char* l1 = convertDecimalBase32 (ic);
	char* l2 = convertDecimalBase32 (dc);
	memset(out,'\0',BASE_32_STRING_LENGTH*2);
	strcat(out," ");
	strcat(out,l1);
	strcat(out," ");
	strcat(out,l2);
	free(l1);
	free(l2);
	return out;
}

char* constructObjectFileLine (int address, char* binary){
	char* out = (char*)malloc(sizeof(char)*BASE_32_STRING_LENGTH*2);
	char* address32 = convertDecimalBase32 (address);
	char* binary32 = convertBinaryStringtoBase32(binary);
	memset(out,'\0',BASE_32_STRING_LENGTH*2);
	strcat(out,address32);
	strcat(out," ");
	strcat(out,binary32);
	free(address32);
	free(binary32);
	return out;
}

char* constructEntExtFileLine (char* labelName, int address){
	char* out = (char*)malloc(sizeof(char)*MAX_LABEL_NAME_LENGTH+5);
	char* num32 = convertDecimalBase32(address);
	memset(out,'\0',BASE_32_STRING_LENGTH*2);
	strcpy(out,labelName);
	strcat(out," ");
	strcat(out,num32);
	free(num32);
	return out;
}
