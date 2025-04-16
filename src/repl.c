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

    // -1 for all, -2 for error, 0+ for success
    if (!extracted || strcmp(extracted, "*") == 0) {
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

  if (strncmp(input, "exit",  4) == 0) {
    command->type = EXIT; 
  }
  return command;
}
