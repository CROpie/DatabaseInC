#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "database.h"

Table* db_open(const char* filename) {
  // int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

  FILE* fp = fopen(filename, "rb+");
  if (!fp) {
    fp = fopen(filename, "wb+");
    if (!fp) {
      printf("Unable to open file.\n");
      exit(EXIT_FAILURE);
    }
  }

  Table* table = (Table*) malloc(sizeof(Table));
  // especially for ensuring row[MAX_ROWS] are all null when accessed for the 1st time
  memset(table, 0, sizeof(Table));
  
  table->fp = fp;
  
  fread(&table->usedRows, sizeof(int), 1, fp);
  fread(&table->capacity, sizeof(int), 1, fp);

  if (!table->capacity) {
    table->capacity = 4;
  }

  // malloc the pointer array
  table->rows = malloc(sizeof(Row*) * table->capacity);
  memset(table->rows, 0, sizeof(Row*) * table->capacity);
  
  return table;
}

void db_close(Table* table) {

  rewind(table->fp);
  fwrite(&table->usedRows, sizeof(int), 1, table->fp);
  fwrite(&table->capacity, sizeof(int), 1, table->fp);

  for (int i = 0; i < table->usedRows; i++) {

    // ignore if NULL because no change
    if (!table->rows[i]) {
      continue;
    }
    
    // calculate where in the file to find this particular row
    long offset = sizeof(int) + sizeof(int) + (i * sizeof(Row));

    fseek(table->fp, offset, SEEK_SET);
    fwrite(table->rows[i], sizeof(Row), 1, table->fp);
    free(table->rows[i]);
  }

  fclose(table->fp);
  free(table);
}

void selectRecord(Table* table, int recordIndex) {

  // check if has been opened in this session
  if (table->rows[recordIndex]) {

    if (!table->rows[recordIndex]->isDeleted) {
      printf("%d) %s\n", recordIndex, table->rows[recordIndex]->message);
    }
    return;
  }

  Row* selectedRow = (Row*) malloc(sizeof(Row));
  memset(selectedRow, 0, sizeof(Row));

  table->rows[recordIndex] = selectedRow;

  // calculate where in the file to find this particular row
  long offset = sizeof(int) + sizeof(int) + (recordIndex * sizeof(Row));

  fseek(table->fp, offset, SEEK_SET);
  fread(selectedRow, sizeof(Row), 1, table->fp);

  if (!selectedRow->isDeleted) {
      printf("%d) %s\n", recordIndex, selectedRow->message);
  }
}

void selectAllRecords(Table* table) {
  for (int i = 0; i < table->usedRows; i++) {
    selectRecord(table, i);
  }
}

void insertRecord(Table* table, Command* command) {
  if (table->usedRows >= table->capacity) {
    int oldCapacity = table->capacity;
    table->capacity *= 2;

    table->rows = realloc(table->rows, sizeof(Row*) * table->capacity);

    // memset only the new part of the array
    memset(&table->rows[oldCapacity], 0, sizeof(Row*) * (table->capacity - oldCapacity));
    printf("Database is full, increasing capacity to %d...!\n", table->capacity);
  }
  Row* newRow = (Row*) malloc(sizeof(Row));
  memset(newRow, 0, sizeof(Row));
  
  newRow->isDeleted = false;
  strcpy(newRow->message, command->message);
  
  table->rows[table->usedRows] = newRow;

  table->usedRows++;

  /*
   Same but harder to read:
  table->rows[table->usedRows] = malloc(sizeof(Row));
  memset(table->rows[table->usedRows], 0, sizeof(Row));
  table->rows[table->usedRows]->isDeleted = false;
  strcpy(table->rows[table->usedRows]->message, command->message);
  */
  printf("Returning..\n");
}

void deleteRecord(Table* table, int recordIndex) {

  // check if has been opened in this session
  if (table->rows[recordIndex]) {
    table->rows[recordIndex]->isDeleted = true;
    return;
  }

  Row* selectedRow = (Row*) malloc(sizeof(Row));
  memset(selectedRow, 0, sizeof(Row));

  table->rows[recordIndex] = selectedRow;

  // calculate where in the file to find this particular row
  long offset = sizeof(int) + sizeof(int) + (recordIndex * sizeof(Row));

  fseek(table->fp, offset, SEEK_SET);
  fread(selectedRow, sizeof(Row), 1, table->fp);

  selectedRow->isDeleted = true;
  table->rows[recordIndex] = NULL;
}

void deleteAllRecords(Table* table) {
  for (int i = 0; i < table->usedRows; i++) {
    deleteRecord(table, i);
  }
}
