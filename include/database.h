#ifndef DATABASE_H
#define DATABASE_H

typedef enum {
  INSERT,
  SELECT,
  EXIT,
  UNDEFINED,
} CommandType;

typedef struct {
  CommandType type;
  char* message;
} Command;

#endif
