#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "repl.h"

// changed this to stderr so it doesn't appear in unit test assertions
void printHome() {
  fprintf(stderr, "db > ");
}

int main() {
  Table* table = db_open();
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
        printf("Executed.\n");
        insertRecord(table, command); 
        break;
      case SELECT:
        if (command->selection == -2) {
          printf("ERROR: not a valid command\n");
          break;
        } else if (command->selection == -1) {
          selectAllRecords(table);
        } else {
          selectRecord(table, command->selection);
        }
        break;
      case DELETE:
        if (command->selection == -2) {
          printf("ERROR: not a valid command\n");
          break;
        } else if (command->selection == -1) {
          printf("Executed.\n");
          deleteAllRecords(table);
        } else {
          printf("Executed.\n");
          deleteRecord(table, command->selection);
        }
        break;
      case EXIT:
        printf("goodbyte\n");
        db_close(table);
        return 0;
      case UNDEFINED:
      default:
        printf("unrecognized command\n");
      }
  }
  db_close(table);
  return 0;
}
