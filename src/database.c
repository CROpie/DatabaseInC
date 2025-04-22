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
  fread(&table->usedRows, sizeof(int), 1, fp);
  fread(&table->pageCapacity, sizeof(int), 1, fp);
  if (!table->pageCapacity) {
    table->pageCapacity = 2;
  }
  table->pages = malloc(sizeof(Page) * table->pageCapacity);
  memset(table->pages, 0, sizeof(Page) * table->pageCapacity);

  fread(table->pages, sizeof(Page), table->pageCapacity, fp);

  fclose(fp);
  
  return table;
}

void db_close(Table* table, const char* filename) {
  FILE* fp = fopen(filename, "wb");
  if (!fp) {
    printf("Unable to open file.\n");
    exit(EXIT_FAILURE);
  }

  fwrite(&table->usedRows, sizeof(int), 1, fp);
  fwrite(&table->pageCapacity, sizeof(int), 1, fp);
  fwrite(table->pages, sizeof(Page), table->pageCapacity, fp);
  free(table->pages);

  fclose(fp);
  free(table);
}

// using direct access
void selectRecord(Table* table, int recordIndex) {
  int currentPage = recordIndex / ROWS_PER_PAGE;  
  int rowOffset = recordIndex % ROWS_PER_PAGE;
  if (!table->pages[currentPage].rows[rowOffset].isDeleted) {
    printf("%d) %s\n", recordIndex, table->pages[currentPage].rows[rowOffset].message);
  }
}

void selectAllRecords(Table* table) {
  for (int i = 0; i < table->usedRows; i++) {
    selectRecord(table, i);
  }
}

void insertRecord(Table* table, Command* command) {
  if (table->usedRows >= ROWS_PER_PAGE * table->pageCapacity) {
    table->pageCapacity *= 2;
    table->pages = realloc(table->pages, table->pageCapacity * sizeof(Page));
  } else {
    int currentPage = table->usedRows / ROWS_PER_PAGE;
    int rowOffset = (table->usedRows % ROWS_PER_PAGE);
    // Row selectedRow = table->pages[currentPage].rows[rowOffset]; doesn't work: makes a copy. Need pointer or direct
    table->pages[currentPage].rows[rowOffset].isDeleted = false;
    strcpy(table->pages[currentPage].rows[rowOffset].message, command->message);
    table->usedRows++;
  }
}

void deleteRecord(Table* table, int recordIndex) {
  int currentPage = recordIndex / ROWS_PER_PAGE;  
  int rowOffset = recordIndex % ROWS_PER_PAGE;
  table->pages[currentPage].rows[rowOffset].isDeleted = true;
}

void deleteAllRecords(Table* table) {
  for (int i = 0; i < table->usedRows; i++) {
    deleteRecord(table, i);
  }
}
