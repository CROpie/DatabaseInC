#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>

#define MESSAGE_LENGTH 64 
#define MAX_ROWS 8

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

typedef struct {
  int usedRows;
  Row rows[MAX_ROWS];
} Table;

Table* db_open(const char* filename);
void db_close(Table* table, const char* filename);
void selectRecord(Table* table, int recordIndex);
void selectAllRecords(Table* table);
void insertRecord(Table* table, Command* command);
void deleteRecord(Table* table, int recordIndex); 
void deleteAllRecords(Table* table); 

#endif
