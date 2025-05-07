#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

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

  fread(&table->usedRows, sizeof(int), 1, fp);
  fread(&table->pageCapacity, sizeof(int), 1, fp);

  if (!table->pageCapacity) {
    table->pageCapacity = 2;
  }

  table->pages = malloc(sizeof(Page*) * table->pageCapacity);
  memset(table->pages, 0, sizeof(Page*) * table->pageCapacity);

  table->fp = fp;

  return table;
}

void db_close(Table* table) {

  rewind(table->fp);

  fwrite(&table->usedRows, sizeof(int), 1, table->fp);
  fwrite(&table->pageCapacity, sizeof(int), 1, table->fp);

  for (int i = 0; i < table->pageCapacity; i++) {
    if (!table->pages[i]) {
      continue;
    }

    long offset = sizeof(int) + sizeof(int) + (i * sizeof(Page));
    fseek(table->fp, offset, SEEK_SET);

    fwrite(table->pages[i], sizeof(Page), 1, table->fp);
    free(table->pages[i]);
  }

  fclose(table->fp);
  free(table->pages);
  free(table);
}

void loadPage(Table* table, int pageNum) {
  Page* loadedPage = (Page*) malloc(sizeof(Page));
  memset(loadedPage, 0, sizeof(Page));
  table->pages[pageNum] = loadedPage;
  printf("Opening page %d\n", pageNum);

  long offset = sizeof(int) + sizeof(int) + (pageNum * sizeof(Page));

  fseek(table->fp, offset, SEEK_SET);
  fread(loadedPage, sizeof(Page), 1, table->fp);
}

Row* getRow(Table* table, int index) {
  int currentPage = index / ROWS_PER_PAGE;
  int rowOffset = index % ROWS_PER_PAGE;
  Row* selectedRow = &table->pages[currentPage]->rows[rowOffset];
  return selectedRow;
}

// using direct access
void selectRecord(Table* table, int recordIndex) {
  int requestedPage = recordIndex / ROWS_PER_PAGE;

  if (!table->pages[requestedPage]) {
    loadPage(table, requestedPage);
  }

  Row* row = getRow(table, recordIndex);
  if (!row->isDeleted) {
    printf("%d) %s\n", recordIndex, row->message);
  }
}

void selectAllRecords(Table* table) {
  for (int i = 0; i < table->usedRows; i++) {
    selectRecord(table, i);
  }
}

void insertRecord(Table* table, Command* command) {
  if (table->usedRows >= ROWS_PER_PAGE * table->pageCapacity) {
    int oldCapacity = table->pageCapacity;
    table->pageCapacity *= 2;
    table->pages = realloc(table->pages, table->pageCapacity * sizeof(Page*));

    // doing this to ensure no uninitialized values when checking for pages
    for (int i = oldCapacity; i < table->pageCapacity; i++) {
      table->pages[i] = NULL;
    }
    printf("Realloc to capacity %d\n", table->pageCapacity);
  }

  int pageNum = table->usedRows / ROWS_PER_PAGE;

  if (!table->pages[pageNum]) {
    loadPage(table, pageNum);
  }

  Row* row = getRow(table, table->usedRows);

  row->isDeleted = false;
  strcpy(row->message, command->message);
  
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
  int requestedPage = recordIndex / ROWS_PER_PAGE;
  if (!table->pages[requestedPage]) {
    loadPage(table, requestedPage);
  }

  Row* row = getRow(table, recordIndex);
  row->isDeleted = true;
}

void deleteAllRecords(Table* table) {
  for (int i = 0; i < table->usedRows; i++) {
    deleteRecord(table, i);
  }
}

void filterRecords(Table* table, Command* command) {

  clock_t start = clock();

  // allocate maximum possible matches in a for a row pointer array on the stack
  Row* matches[table->usedRows];
  int numMatches = 0;

  // iterate over rows, adding to usedRows
  for (int i = 0; i < table->usedRows; i++) {
    int requestedPage = i / ROWS_PER_PAGE;
  
    if (!table->pages[requestedPage]) {
      loadPage(table, requestedPage);
    }
    Row* row = getRow(table, i);
     
    if (strstr(row->message, command->message)) {
      printf("found match in %s\n", row->message);
      matches[numMatches++] = row;
    }
  }
  clock_t end = clock();
  double timeSpent = (double) (end - start) / CLOCKS_PER_SEC;

  printf("Found %d matches in  %.6f seconds\n", numMatches, timeSpent);
}
