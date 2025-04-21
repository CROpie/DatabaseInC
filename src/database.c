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
  
  return table;
}

void db_close(Table* table) {

  rewind(table->fp);
  fwrite(&table->usedRows, sizeof(int), 1, table->fp);

  for (int i = 0; i < table->usedRows; i++) {

    // ignore if NULL because no change
    if (!table->rows[i]) {
      continue;
    }
    
    // calculate where in the file to find this particular row
    long offset = sizeof(int) + (i * sizeof(Row));

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
  long offset = sizeof(int) + (recordIndex * sizeof(Row));

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
  if (table->usedRows >= MAX_ROWS) {
    printf("Database is full!\n");
    return;
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
  long offset = sizeof(int) + (recordIndex * sizeof(Row));

  fseek(table->fp, offset, SEEK_SET);
  fread(selectedRow, sizeof(Row), 1, table->fp);

  selectedRow->isDeleted = true;
}

void deleteAllRecords(Table* table) {
  for (int i = 0; i < table->usedRows; i++) {
    deleteRecord(table, i);
  }
}
