#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "database.h"
#include "repl.h"
#include "btree.h"

void freeCommand(Command* command) {
  if (!command) return;

  if (command->message) {
    free(command->message);
  }

  free(command);
}

// changed this to stderr so it doesn't appear in unit test assertions
void printHome() {
  fprintf(stderr, "db > ");
}

int main() {
  clock_t start = clock();
  Table* table = db_open("data.db");
  Tree* tree = loadTree("index.db");
  if (!tree) {
    db_close(table);
    return 1;
  }
  while (1) {
    printHome();
    char* input = getInput();
    if (input == NULL) {
      printf("Failed to get input\n");
      db_close(table);
      return 1;
    }
    Command* command = parseInput(input);
    free(input);
    switch (command->type) {
      case INSERT:
        printf("Executed.\n");
        insertRecord(table, command); 
        insertIntoTree(tree, table->usedRows, command->message);
        break;
      case SELECT:
        if (command->selection == -2) {
          printf("ERROR: not a valid command\n");
        } else if (command->selection == -1) {
          selectAllRecords(table);
        } else {
          selectRecord(table, command->selection);
        }
        break;
      case DELETE:
        if (command->selection == -2) {
          printf("ERROR: not a valid command\n");
        } else if (command->selection == -1) {
          printf("Executed.\n");
          deleteAllRecords(table);
        } else {
          printf("Executed.\n");
          deleteRecord(table, command->selection);
        }
        break;
      case FILTER:
        if (command->selection == -2) {
          printf("ERROR: not a valid command\n");
        } else {
          printf("Executed.\n");
          filterRecords(table, command);
        }
        break;
      case SEARCH:
        if (command->selection == -2) {
          printf("ERROR: not a valid command\n");
        } else {
          printf("Executed.\n");
          searchString(tree, command->message);
        }
        break;
      case EXIT:
        printf("goodbyte\n");
        freeCommand(command);
        db_close(table);
        closeTree(tree);
        clock_t end = clock();
        double timeSpent = (double) (end - start) / CLOCKS_PER_SEC;
        printf("Time: %.6f seconds\n", timeSpent);
        return 0;
      case UNDEFINED:
      default:
        printf("unrecognized command\n");
      }
    freeCommand(command);
  }
  db_close(table);
  return 1;
}
