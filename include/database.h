#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>

#define MESSAGE_LENGTH 64 
#define PAGE_SIZE 512

typedef enum {
  INSERT,
  SELECT,
  DELETE,
  EXIT,
  UNDEFINED,
} CommandType;

// selection: number of a row, or -1 for all
// used in select and delete
typedef struct {
  CommandType type;
  char* message;
  int selection;
} Command;

typedef struct {
  char message[MESSAGE_LENGTH];
  bool isDeleted; 
} Row;

#define ROWS_PER_PAGE (PAGE_SIZE / sizeof(Row))
#define MAX_ROWS (MAX_PAGES * ROWS_PER_PAGE)

typedef struct {
  Row rows[ROWS_PER_PAGE];
} Page;

typedef struct {
  int usedRows;
  int pageCapacity;
  Page* pages;
} Table;


Table* db_open(const char* filename);
void db_close(Table* table, const char* filename);
void selectRecord(Table* table, int recordIndex);
void selectAllRecords(Table* table);
void insertRecord(Table* table, Command* command);
void deleteRecord(Table* table, int recordIndex); 
void deleteAllRecords(Table* table); 

#endif
