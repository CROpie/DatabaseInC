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
  fread(table, sizeof(Table), 1, fp);
  fclose(fp);
  
  return table;
}

void db_close(Table* table, const char* filename) {
  FILE* fp = fopen(filename, "wb");
  if (!fp) {
    printf("Unable to open file.\n");
    exit(EXIT_FAILURE);
  }

  fwrite(table, sizeof(Table), 1, fp);

  fclose(fp);
  free(table);
}

// using direct access
void selectRecord(Table* table, int recordIndex) {
    if (!table->rows[recordIndex].isDeleted) {
      printf("%d) %s\n", recordIndex, table->rows[recordIndex].message);
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
  } else {
    table->rows[table->usedRows].isDeleted = false;
    strcpy(table->rows[table->usedRows].message, command->message);
    table->usedRows++;
  }
}

void deleteRecord(Table* table, int recordIndex) {
    table->rows[recordIndex].isDeleted = true;
}

void deleteAllRecords(Table* table) {
  for (int i = 0; i < table->usedRows; i++) {
    deleteRecord(table, i);
  }
}
