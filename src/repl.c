#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "repl.h"
#include "database.h"

char* getInput() {
  size_t size = INPUT_LENGTH;
  char* buffer = malloc(size);
  fgets(buffer, size, stdin);

  if (strchr(buffer, '\n') == NULL) {
    printf("Input was too long and got cut off.\n");
    free(buffer);
    return NULL;
  }

  return buffer;
}

char* truncateInput(int N, char* input) {
  int len = strlen(input);  
  char* truncated = malloc(len - N);
  for (int i = 0; N < len; N++, i++) {
    truncated[i] = input[N];
  }
  return truncated;
}

Command* parseInput(char* input) {
  Command* command = (Command*) malloc(sizeof(Command));
  command->type = UNDEFINED;
  if (strncmp(input, "insert",  6) == 0) {
    command->type = INSERT;
    char* extracted = truncateInput(7, input);
    command->message = extracted;
  }
  if (strncmp(input, "select",  6) == 0) {
    command->type = SELECT; 
    char* extracted = truncateInput(7, input);
    command->message = extracted;
  }
  if (strncmp(input, "exit",  4) == 0) {
    command->type = EXIT; 
  }
  return command;
}
