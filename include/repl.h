#ifndef REPL_H
#define REPL_H

#include "database.h"

#define INPUT_LENGTH 25

char* getInput(); 
char* truncateInput(int N, char* input);
Command* parseInput(char* input);

#endif
