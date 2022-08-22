/*
 * preprocessor.h
 * 		module is in charge of initially reading the .as file and replacing macro statements saving
 * 		the code to .am file
 */
#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

/*
 * Tries to read the file "[name].as", treats macro declarations
 * and saves the completed file as file "[name].am"
 */
void preprocessor(char *name, int *success);

#endif
