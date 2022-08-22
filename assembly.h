/*
 * assembly.h
 * 		module provides a function to compile a given assembler code file into object, entry and extern files
 */
#ifndef ASSEMBLY_H
#define ASSEMBLY_H
#include "data.h"

/*
 * Manages the assembly process.
 * Receives a .am file name (without extension), performs validation and compiling on the file
 * If file is valid the function writes the compiled object, entry and extern files
 * Returns 1 if succeeded or 0 if encountered errors
 */
int assemble (char *name);

#endif
