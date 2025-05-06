#ifndef BTREE_H
#define BTREE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define LEAF_MAX 4
#define INTERNAL_MAX 4 
#define KEY_LENGTH 64 

typedef struct {
  void* root;
  int idCounter;
  int rootId;
  FILE* fp;
} Tree;

typedef struct {
  char key[KEY_LENGTH];
  int pageNum;
  int rowOffset;
} Entry;

typedef struct {
  int isLeaf;
  int id;
  int numEntries;
  Entry entries[LEAF_MAX];
} LeafNode;

typedef struct {
  int isLeaf;
  int id;
  int numKeys;
  char keys[INTERNAL_MAX][KEY_LENGTH];
  int childIds[INTERNAL_MAX + 1];
  void* children[INTERNAL_MAX + 1];
} InternalNode;

Tree* loadTree(const char* filename);
void insertIntoTree(Tree* tree, int index, char* string);
void serializeTree(Tree* tree);
void searchString(Tree* tree, char* string);
#endif
