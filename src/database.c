#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "database.h"

Table* db_open(const char* filename) {
  // int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    fp = fopen(filename, "wb+");
    if (!fp) {
      printf("Unable to open file.\n");
      exit(EXIT_FAILURE);
    }
  }

  Table* table = (Table*) malloc(sizeof(Table));
  memset(table, 0, sizeof(Table));
  
  fread(&table->capacity, sizeof(int), 1, fp);

  // initialize database capacity
  if (!table->capacity) {
    table->capacity = 4;
  }
  
  // allocate rows since they are now dynamic
  table->rows = malloc(sizeof(Row) * table->capacity); 


  // initialize database capacity
  if (!table->capacity) {
    table->capacity = 4;
  }
  fread(&table->usedRows, sizeof(int), 1, fp);
  fread(table->rows, sizeof(Row), table->usedRows, fp);

  printf("init capacity: %d, used: %d\n", table->capacity, table->usedRows);
  rewind(fp);
  
  fclose(fp);
  
  return table;
}

void db_close(Table* table, const char* filename) {
  FILE* fp = fopen(filename, "wb");
  if (!fp) {
    printf("Unable to open file.\n");
    exit(EXIT_FAILURE);
  }
  rewind(fp);

  fwrite(&table->capacity, sizeof(int), 1, fp);
  fwrite(&table->usedRows, sizeof(int), 1, fp);
  fwrite(table->rows, sizeof(Row), table->usedRows, fp);
  fclose(fp);
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
  if (table->usedRows >= table->capacity) {
    printf("Database was full!\n");
    table->capacity *= 2;
    table->rows = realloc(table->rows, table->capacity * sizeof(Row));
    printf("realloc capacity: %d, used: %d\n", table->capacity, table->usedRows);
    if (table->rows == NULL) {
      printf("Failure to reallocate rows\n");
      exit(EXIT_FAILURE);
    }
  }
  Row* newRow = (Row*) malloc(sizeof(Row));
  memset(newRow, 0, sizeof(Row));
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
