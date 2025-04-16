#ifndef REPL_H
#define REPL_H

#include "database.h"

#define INPUT_LENGTH (MESSAGE_LENGTH - 7)

char* getInput(); 
char* truncateInput(int N, char* input);
Command* parseInput(char* input);

#endif
