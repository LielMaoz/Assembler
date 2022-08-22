/*
 * OutputFormatter.h
 *	module in charge of constructing the different strings of decoded data
 *	and formatting it for the file output in the special base 32
 *
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_
#include "constraints.h"
/*
 * 	Formats a machine code instruction in binary
 */
char* constructType1Binary (int opCode, int typeOpSrc, int typeOpDst, int are);

/*
 * Formats the added word needed for Labels and Symbols in binary
 */
char* constructType2Binary (int value, int are);

/*
 * Formats the added word needed for immediate values or indexes in binary
 */
char* constructType3Binary (int value);

/*
 * Formats the added word needed for register addresses in binary
 */
char* constructType4Binary (int regSrc, int regDst);

/*
 * Receives a string of a 10 bit binary sequence and returns a converted base 32 string
 */
char* convertBinaryStringtoBase32 (char* binary);

/*
 * Receives a decimal integer and returns a converted base 32 string
 */
char* convertDecimalBase32(int decimalNum);

char* constructObjectFileFirstLine (int ic, int dc);

char* constructObjectFileLine (int address, char* binary);

char* constructEntExtFileLine (char* LabelName, int address);
#endif /* MAMAN14_OUTPUTFORMATTER_H_ */
