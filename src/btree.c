#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <btree.h>
#include <database.h>

int compare_entries(const void* pa, const void* pb) {
  const Entry *a = (const Entry*)pa;
  const Entry *b  = (const Entry*)pb;
  return strcmp(a->key, b->key);
};

void printArray(LeafNode* leafNode) {
  printf("LEAF: %p\n", leafNode);
  printf("{ ");
  for (int i = 0; i < leafNode->numEntries; i++) {
    printf("%s ", leafNode->entries[i].key);
  }
  printf("}\n");
}

void printInternal(InternalNode* internalNode) {
  printf("INTERNAL: %p\n", internalNode);
  printf("{ ");
  for (int i = 0; i < internalNode->numKeys; i++) {
    printf("%s ", internalNode->keys[i]);
  }
  printf("}\n");
}

InternalNode* createInternal(Tree* tree, void* left, void* right, char* promotionKey) {
  InternalNode* newInternal = (InternalNode*) malloc(sizeof(InternalNode));
  memset(newInternal, 0, sizeof(InternalNode));
  newInternal->isLeaf = 0;
  newInternal->id = tree->idCounter++;
  strcpy(newInternal->keys[0], promotionKey);
  newInternal->numKeys = 1;
  newInternal->children[0] = left;
  newInternal->children[1] = right;

  newInternal->childIds[0] = ((InternalNode*)left)->id;
  newInternal->childIds[1] = ((InternalNode*)right)->id;

  // initialize unused children to NULL
  for (int i = 2; i <= INTERNAL_MAX; i++) {
    newInternal->children[i] = NULL;
  }
  return newInternal;
}

InternalNode* findParent(void* root, void* child, char* key) {

  // no parent yet
  if (root == child) {
    return NULL;
  }

  InternalNode* internalNode = (InternalNode*)root;

  // base case: iterate over children of current node to see if match child
  for (int i = 0; i <= internalNode->numKeys; i++) {
    if (((InternalNode*)internalNode->children[i]) == (InternalNode*)child) {
      return internalNode;
    } 
  }

  int childIndex = 0;
  while (childIndex < internalNode->numKeys && strcmp(key, internalNode->keys[childIndex]) >= 0) {
    childIndex++;
  }
  
  return findParent(internalNode->children[childIndex], child, key);
}

void addToParent(InternalNode* internal, void* left, void* right, char* promotionKey) {

  // find where to insert the key
  int insertIndex = 0;
  int numKeys = internal->numKeys;
  while (insertIndex < numKeys && strcmp(promotionKey, internal->keys[insertIndex]) > 0) {
    insertIndex++;
  }

  // shift keys and children to make space
  for (int i = numKeys; i > insertIndex; i--) {
    strcpy(internal->keys[i], internal->keys[i-1]);
  }

  for (int i = numKeys + 1; i > insertIndex + 1; i--) {
    internal->children[i] = internal->children[i - 1];
    internal->childIds[i] = internal->childIds[i - 1];
  }

  // insert new key and new child pointers
  strcpy(internal->keys[insertIndex], promotionKey);
  internal->children[insertIndex] = left;
  internal->children[insertIndex + 1] = right;

  internal->childIds[insertIndex] = ((InternalNode*)left)->id;
  internal->childIds[insertIndex + 1] = ((InternalNode*)right)->id;

  internal->numKeys++;
}

char* splitInternal(Tree* tree, InternalNode* internal, InternalNode* left, InternalNode* right) {
  int intHalf = INTERNAL_MAX / 2;

  left->isLeaf = 0;
  right->isLeaf = 0;

  left->id = tree->idCounter++;
  right->id = tree->idCounter++;

  left->numKeys = intHalf;
  right->numKeys = intHalf - 1;

  memcpy(left->keys, internal->keys, intHalf * KEY_LENGTH); 
  memcpy(right->keys, internal->keys + intHalf + 1, (intHalf - 1) * KEY_LENGTH);

  memcpy(left->children, internal->children, (intHalf + 1) * sizeof(void*)); 
  memcpy(right->children, internal->children + intHalf + 1, ((intHalf - 1) + 1) * sizeof(void*));

  memcpy(left->childIds, internal->childIds, (intHalf + 1) * sizeof(int));
  memcpy(right->childIds, internal->childIds + intHalf + 1, ((intHalf - 1) + 1) * sizeof(int));

  char* promotionKey = internal->keys[intHalf];

  return promotionKey; 
}

InternalNode* promotionCycle(Tree* tree, void* preSplit, void* left, void* right, char* promotionKey) {
  InternalNode* parent = findParent(tree->root, preSplit, promotionKey);

  // free(preSplit); // what about when preSplit == a different pointer (root?)

  if (!parent) {
    return createInternal(tree, left, right, promotionKey);
  }

  // add, but also update pointers
  addToParent(parent, left, right, promotionKey);

  if (parent->numKeys < INTERNAL_MAX) {
    return tree->root;
  }

  // split parent
  InternalNode* leftInternal = (InternalNode*) malloc(sizeof(InternalNode));
  InternalNode* rightInternal = (InternalNode*) malloc(sizeof(InternalNode));

  memset(leftInternal, 0, sizeof(InternalNode));
  memset(rightInternal, 0, sizeof(InternalNode));

  // promotionIndex is removed from an internal node
  char* nextPromotionIndex = splitInternal(tree, parent, leftInternal, rightInternal);

  // printInternal(leftInternal);
  // printInternal(rightInternal);

  return promotionCycle(tree, parent, leftInternal, rightInternal, nextPromotionIndex);
}

char* splitLeaf(Tree* tree, LeafNode* leaf, LeafNode* left, LeafNode* right) {
  int half = LEAF_MAX / 2;

  left->isLeaf = 1;
  right->isLeaf = 1;

  left->id = tree->idCounter++;
  right->id = tree->idCounter++;

  left->numEntries = half;
  right->numEntries = half;

  memcpy(left->entries, leaf->entries, half * sizeof(Entry)); 
  memcpy(right->entries, leaf->entries + half, half * sizeof(Entry));
  
  return right->entries[0].key;
}

void addToLeafAndSort(LeafNode* leaf, Entry* entry) {
  // add to end of array
  leaf->entries[leaf->numEntries++] = *entry;
  
  // sort array
  qsort(leaf->entries, leaf->numEntries, sizeof(Entry), compare_entries);
}

LeafNode* createFirstLeaf(Tree* tree, Entry* entry) {
  LeafNode* newRoot = (LeafNode*) malloc(sizeof(LeafNode));
  memset(newRoot, 0, sizeof(LeafNode));
  newRoot->isLeaf = 1;
  newRoot->id = tree->idCounter++;
  newRoot->entries[0] = *entry;
  newRoot->numEntries = 1;
  return newRoot;
}

LeafNode* findLeaf(void* node, char* key) {

  if (!node) {
    return NULL;
  }

  // cast the node to internal even if it isn't one. Allows checking of the isLeaf attribute
  InternalNode* internalNode = (InternalNode*)node;

  // Base case: any leaf of the current internal node is found, return the internal node
  if (internalNode->isLeaf) {
    return (LeafNode*)node;
  }

  int childIndex = 0;
  while (childIndex < internalNode->numKeys && strcmp(key, internalNode->keys[childIndex]) >= 0) {
    childIndex++;
  }

  return findLeaf(internalNode->children[childIndex], key);
}

void* insert(Tree* tree, Entry* entry) {

  LeafNode* leaf = findLeaf(tree->root, entry->key);

  if (!leaf) {
    return createFirstLeaf(tree, entry);
  }

  addToLeafAndSort(leaf, entry);

  // check if split needed
  if (leaf->numEntries < LEAF_MAX) {
    return tree->root;
  }

  // split leaf node
  LeafNode* leftLeaf = (LeafNode*) malloc(sizeof(LeafNode));
  LeafNode* rightLeaf = (LeafNode*) malloc(sizeof(LeafNode));

  memset(leftLeaf, 0, sizeof(LeafNode));
  memset(rightLeaf, 0, sizeof(LeafNode));
  // promotionIndex remains in a leaf
  char* promotionKey = splitLeaf(tree, leaf, leftLeaf, rightLeaf);

  // promote, split if required, repeat if necessary
  return promotionCycle(tree, leaf, leftLeaf, rightLeaf, promotionKey);
}

void printTree(void* node, int level) {
    if (node == NULL) return;

    if (((InternalNode*)node)->isLeaf) {
        LeafNode* leaf = (LeafNode*)node;
        printf("%*sLeaf: ", level * 2, "");
        for (int i = 0; i < leaf->numEntries; i++) {
            printf("%d: %s ", leaf->entries[i].pageNum, leaf->entries[i].key);
        }
        printf("\n");
    } else {
        InternalNode* internal = (InternalNode*)node;
        printf("%*sInternal: ", level * 2, "");
        for (int i = 0; i < internal->numKeys; i++) {
            printf("%s ", internal->keys[i]);
        }
        printf("\n");

        for (int i = 0; i <= internal->numKeys; i++) {
            printTree(internal->children[i], level + 1);
        }
    }
}

void traverseAllNodes(void* node) {
  if (node == NULL) return;

  if (((LeafNode*)node)->isLeaf) {
    LeafNode* leaf = (LeafNode*)node;
    printf("Leaf node (%d) with %d entries\n", leaf->id, leaf->numEntries);
    printf("[");
    for (int i = 0; i < leaf->numEntries; i++) {
      printf("%s ", leaf->entries[i].key);
    }
    printf("]\n");
  } else {
    InternalNode* internal = (InternalNode*)node;
    printf("Internal node (%d) with %d keys\n", internal->id, internal->numKeys);

    printf("[");
    for (int i = 0; i < internal->numKeys; i++) {
      printf("%s ", internal->keys[i]);
    }
    printf("]\n");

    for (int i = 0; i < internal->numKeys + 1; i++) {
      printf("child id: %d @ %p\n", internal->childIds[i], internal->children[i]);
    }

    for (int i = 0; i <= internal->numKeys; i++) {
      traverseAllNodes(internal->children[i]);
    }
  }
}

void saveInternalNode(FILE* fp, InternalNode* internal) {
  fwrite(&internal->isLeaf, sizeof(int), 1, fp);
  fwrite(&internal->id, sizeof(int), 1, fp);
  fwrite(&internal->numKeys, sizeof(int), 1, fp);
  fwrite(internal->keys, sizeof(char), internal->numKeys * KEY_LENGTH, fp);
  fwrite(internal->childIds, sizeof(int), internal->numKeys + 1, fp);
}

void saveLeafNode(FILE* fp, LeafNode* leaf) {
  fwrite(&leaf->isLeaf, sizeof(int), 1, fp);
  fwrite(&leaf->id, sizeof(int), 1, fp);
  fwrite(&leaf->numEntries, sizeof(int), 1, fp);
  fwrite(leaf->entries, sizeof(Entry), leaf->numEntries, fp);
}

void saveByTraverse(FILE* fp, void* node) {
  if (node == NULL) return;

  if (((LeafNode*)node)->isLeaf) {
    LeafNode* leaf = (LeafNode*)node;
    saveLeafNode(fp, leaf);
  } else {
    InternalNode* internal = (InternalNode*)node;
    int childIndexes[internal->numKeys + 1];
    for (int i = 0; i <= internal->numKeys; i++) {
      saveByTraverse(fp, internal->children[i]);
    }
    saveInternalNode(fp, internal);
  }
}

void serializeTree(Tree* tree) {
  rewind(tree->fp);
  fwrite(&tree->idCounter, sizeof(int), 1, tree->fp);
  fwrite(&tree->rootId, sizeof(int), 1, tree->fp);
  
  saveByTraverse(tree->fp, tree->root);
  fclose(tree->fp);
}

LeafNode* loadLeafNode(FILE* fp) {
  LeafNode* leaf = malloc(sizeof(LeafNode));
  fread(&leaf->id, sizeof(int), 1, fp);
  fread(&leaf->numEntries, sizeof(int), 1, fp);
  fread(&leaf->entries, sizeof(Entry), leaf->numEntries, fp);
  leaf->isLeaf = 1;
  return leaf;
}

InternalNode* loadInternalNode(FILE* fp) {
  InternalNode* internal = malloc(sizeof(InternalNode));
  memset(internal, 0, sizeof(InternalNode));
  fread(&internal->id, sizeof(int), 1, fp);
  fread(&internal->numKeys, sizeof(int), 1, fp);
  fread(&internal->keys, sizeof(char), internal->numKeys * KEY_LENGTH, fp);
  fread(&internal->childIds, sizeof(int), internal->numKeys + 1, fp);
  internal->isLeaf = 0;
//  printf("Internal %d\n", internal->id);
//    for (int i = 0; i < internal->numKeys + 1; i++) {
//      printf("child id: %d @ %p\n", internal->childIds[i], internal->children[i]);
//    }
  return internal;
}

Tree* loadTree(const char* filename) {
  FILE* fp = fopen(filename, "rb+");
  if (!fp) {
    fp = fopen(filename, "wb+");
    if (!fp) {
      printf("Unable to open file.\n");
      exit(EXIT_FAILURE);
    }
  }
  Tree* tree = malloc(sizeof(Tree));
  memset(tree, 0, sizeof(Tree));
  fread(&tree->idCounter, sizeof(int), 1, fp);
  fread(&tree->rootId, sizeof(int), 1, fp);

  tree->fp = fp;

  if (!tree->idCounter) {
    tree->idCounter = 1;
  }
    
  // InternalNode** nodes = malloc(sizeof(InternalNode*) * tree->idCounter + 1);
  InternalNode** nodes = calloc(tree->idCounter + 1, sizeof(InternalNode*));
  

  // should be while loop instead of for
  // don't want to have to store number of nodes
  // and idCounter != numNodes since nodes can be deleted
  for (int i = 1; i < tree->idCounter; i++) {
    int isLeaf;
    fread(&isLeaf, sizeof(int), 1, fp);
    if (isLeaf) {
      LeafNode* leaf = loadLeafNode(fp);
      nodes[leaf->id] = (InternalNode*) leaf;
    } else {
      InternalNode* internal = loadInternalNode(fp);
      nodes[internal->id] = internal;
    }
  }

  for (int i = 1; i < tree->idCounter; i++) {
    InternalNode* internal = (InternalNode*) nodes[i];
    if (!internal) {
      continue;
    }
    if (!internal->isLeaf) {
      for (int j = 0; j < internal->numKeys + 1; j++) {
        internal->children[j] = nodes[internal->childIds[j]];
      }
    }
  }

  tree->root = nodes[tree->rootId];
  // traverseAllNodes(tree->root);
  free(nodes);
  return tree;
}

Entry* createEntry(int index, char* string) {
  Entry* entry = malloc(sizeof(Entry));
  memset(entry, 0, sizeof(Entry));
  entry->pageNum = index / ROWS_PER_PAGE;
  entry->rowOffset = index % ROWS_PER_PAGE;
  strcpy(entry->key, string);
  return entry;
}

void insertIntoTree(Tree* tree, int index, char* string) {
  Entry* entry = createEntry(index, string);
  void* root = insert(tree, entry);
  if (root) {
    tree->rootId = ((InternalNode*)root)->id;
  }
  tree->root = root;
}

LeafNode* findFirstLeafPrefixMatch(void* node, char* key, int* depth) {

  if (!node) {
    return NULL;
  }

  // cast the node to internal even if it isn't one. Allows checking of the isLeaf attribute
  InternalNode* internalNode = (InternalNode*)node;

  // Base case: any leaf of the current internal node is found, return the internal node
  if (internalNode->isLeaf) {
    return (LeafNode*)node;
  }

  (*depth)++;
  int childIndex = 0;

  while (childIndex < internalNode->numKeys && strncmp(key, internalNode->keys[childIndex], strlen(key)) >= 0) {
    childIndex++;
  }


  return findFirstLeafPrefixMatch(internalNode->children[childIndex], key, depth);
}

void searchString(Tree* tree, char* string) {
  int depth = 0;
  LeafNode* leaf = findFirstLeafPrefixMatch(tree->root, string, &depth);
  printf("depth: %d\n", depth);
  char* match = NULL; 
  for (int i = 0; i < leaf->numEntries; i++) {
    if (strncmp(string, leaf->entries[i].key, strlen(string)) == 0) {
      match = leaf->entries[i].key;
      break;
    }
  }
  if (match) {
    printf("%s\n", match);
  } else {
    printf("NO MATCH\n");
  }
}


