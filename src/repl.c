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
    // Discard remaining input in stdin
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    printf("Input was too long and got cut off.\n");
    free(buffer);
    return NULL;
  }

  // remove newline if it exists
  buffer[strcspn(buffer, "\n")] = '\0';

  return buffer;
}

int parseTruncate(char* extracted) {
  char *endptr;
  long int result = strtol(extracted, &endptr, 10);
  if (endptr == extracted) {
    printf("No digits found\n");
    result = -2;
  } else if (*endptr != '\0') {
    printf("Invalid character: %c\n", *endptr);
    result = -2;
  }

  return result;
}
 
/*
Looking at multiple versions of truncating a string

input: eg "insert hello\0" -> "hello\0"

char* truncated = input + 7
trucated is a refernce to the input string. If input is freed or modified then trucated is invalid

making a copy of the string in a different memory location
char* truncated = malloc(strlen(input + 7) + 1);
strcpy(truncated, input + 7); (copy "hello\0)
-> NOTE:  strlen(input + 7); isn't eg strlen(13), it is the length starting from the 7th index

str function which does auto-malloc:
char* truncated = strdup(&input[7]); 

*/

char* truncateInput(int N, char* input) {
  char* truncated = malloc(strlen(input + N) + 1);
  strcpy(truncated, input + 7);
  free(input);
  return truncated;
}

char* truncateInputAutoVersion(int N, char* input) {
  char* truncated = strdup(&input[7]);
  free(input);
  return truncated;
}

// input: eg "insert hello\n" -> "hello\0\0"
// ** need to comment out the remove newline in  getInput() for this to work
char* truncateInputManualVer(int N, char* input) {
  int len = strlen(input); 
  int truncLen = len - N + 1; // +1 for null terminator
  char* truncated = malloc(truncLen);
  for (int i = 0; N < len; N++, i++) {
    truncated[i] = input[N];
  }
  // ensure string is null terminated
  truncated[truncLen - 1] = '\0';
  
  // replace newline with null
  truncated[strcspn(truncated, "\n")] = '\0'; 
  free(input);
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

    // -1 for all, -2 for error, 0+ for success
    if (!extracted || strcmp(extracted, "*") == 0 || strlen(extracted) == 0) {
      command->selection = -1;
      return command;
    } else {
      command->selection = parseTruncate(extracted);
    }
  }

  if (strncmp(input, "delete",  6) == 0) {
    command->type = DELETE; 
    char* extracted = truncateInput(7, input);

    // -1 for all, -2 for error, 0+ for success
    if (!extracted || strcmp(extracted, "*") == 0) {
      command->selection = -1;
      return command;
    } else {
      command->selection = parseTruncate(extracted);
    }
  }

  if (strncmp(input, "filter",  6) == 0) {
    command->type = FILTER;
    char* extracted = truncateInput(7, input);
    command->message = extracted;
  }

  if (strncmp(input, "search",  6) == 0) {
    command->type = SEARCH;
    char* extracted = truncateInput(7, input);
    command->message = extracted;
  }

  if (strncmp(input, "exit",  4) == 0) {
    command->type = EXIT; 
  }
  return command;
}
