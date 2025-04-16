#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "database.h"

Table* db_open() {
  Table* table = (Table*) malloc(sizeof(Table));
  table->usedRows = 0;
  return table;
}

void db_close(Table* table) {
  free(table);
}

void selectRecord(Table* table, int recordIndex) {
    Row* currentRow = &(table->rows[recordIndex]);
    if (!currentRow->isDeleted) {
      printf("%d) %s\n", recordIndex, currentRow->message);
    }
}

void selectAllRecords(Table* table) {
  for (int i = 0; i < table->usedRows; i++) {
    selectRecord(table, i);
  }
}

void insertRecord(Table* table, Command* command) {
  if (table->usedRows == MAX_ROWS) {
    printf("Database is full!\n");
    return;
  }
  Row* newRow = (Row*) malloc(sizeof(Row));
  newRow->isDeleted = false;
  strcpy(newRow->message, command->message);
  table->rows[table->usedRows] = *newRow;
  table->usedRows++;
}

void deleteRecord(Table* table, int recordIndex) {
    Row* currentRow = &(table->rows[recordIndex]);
    if (!currentRow->message) {
      printf("Nothing to delete!\n");
    }
    currentRow->isDeleted = true;
}

void deleteAllRecords(Table* table) {
  for (int i = 0; i < table->usedRows; i++) {
    deleteRecord(table, i);
  }
}
