#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "repl.h"


void printHome() {
  printf("db > ");
}

int main() {
  while (1) {
    printHome();
    char* input = getInput();
    if (input == NULL) {
      printf("Failed to get input\n");
      return 1;
    }
    Command* command = parseInput(input);
    switch (command->type) {
      case INSERT:
        printf("running insert\n");
        break;
      case SELECT:
        printf("running select\n");
        break;
      case EXIT:
        printf("goodbyte\n");
        return 0;
      case UNDEFINED:
      default:
        printf("unrecognized command\n");
      }
  }
  return 0;
}
