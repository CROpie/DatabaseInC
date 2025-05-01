#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define LEAF_MAX 4
#define INTERNAL_MAX 4 
#define KEY_LENGTH 17 

typedef struct {
  char key[KEY_LENGTH];
  int pageNum;
  int rowOffset;
} Entry;

typedef struct {
  int isLeaf;
  int numEntries;
  Entry entries[LEAF_MAX];
} LeafNode;

typedef struct {
  int isLeaf;
  int numKeys;
  char keys[INTERNAL_MAX][KEY_LENGTH];
  void* children[INTERNAL_MAX + 1];
} InternalNode;

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

InternalNode* createInternal(void* left, void* right, char* promotionKey) {
  InternalNode* newInternal = (InternalNode*) malloc(sizeof(InternalNode));
  newInternal->isLeaf = 0;
  strcpy(newInternal->keys[0], promotionKey);
  newInternal->numKeys = 1;
  newInternal->children[0] = left;
  newInternal->children[1] = right;

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
  }

  // insert new key and new child pointers
  strcpy(internal->keys[insertIndex], promotionKey);
  internal->children[insertIndex] = left;
  internal->children[insertIndex + 1] = right;

  internal->numKeys++;
}

char* splitInternal(InternalNode* internal, InternalNode* left, InternalNode* right) {
  int intHalf = INTERNAL_MAX / 2;

  left->isLeaf = 0;
  right->isLeaf = 0;

  left->numKeys = intHalf;
  right->numKeys = intHalf - 1;

  memcpy(left->keys, internal->keys, intHalf * KEY_LENGTH); 
  memcpy(right->keys, internal->keys + intHalf + 1, (intHalf - 1) * KEY_LENGTH);

  memcpy(left->children, internal->children, (intHalf + 1) * sizeof(void*)); 
  memcpy(right->children, internal->children + intHalf + 1, ((intHalf - 1) + 1) * sizeof(void*));

  char* promotionKey = internal->keys[intHalf];

  return promotionKey; 
}

InternalNode* promotionCycle(void* root, void* preSplit, void* left, void* right, char* promotionKey) {
  InternalNode* parent = findParent(root, preSplit, promotionKey);

  // free(preSplit); // what about when preSplit == a different pointer (root?)

  if (!parent) {
    return createInternal(left, right, promotionKey);
  }

  // add, but also update pointers
  addToParent(parent, left, right, promotionKey);

  if (parent->numKeys < INTERNAL_MAX) {
    return root;
  }

  // split parent
  InternalNode* leftInternal = (InternalNode*) malloc(sizeof(InternalNode));
  InternalNode* rightInternal = (InternalNode*) malloc(sizeof(InternalNode));

  // promotionIndex is removed from an internal node
  char* nextPromotionIndex = splitInternal(parent, leftInternal, rightInternal);

  printInternal(leftInternal);
  printInternal(rightInternal);

  return promotionCycle(root, parent, leftInternal, rightInternal, nextPromotionIndex);
}

char* splitLeaf(LeafNode* leaf, LeafNode* left, LeafNode* right) {
  int half = LEAF_MAX / 2;

  left->isLeaf = 1;
  right->isLeaf = 1;

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

LeafNode* createFirstLeaf(Entry* entry) {
  LeafNode* newRoot = (LeafNode*) malloc(sizeof(LeafNode));
  newRoot->isLeaf = 1;
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

void* insert(void* root, Entry* entry) {

  LeafNode* leaf = findLeaf(root, entry->key);

  if (!leaf) {
    return createFirstLeaf(entry);
  }

  addToLeafAndSort(leaf, entry);

  // check if split needed
  if (leaf->numEntries < LEAF_MAX) {
    return root;
  }

  // split leaf node
  LeafNode* leftLeaf = (LeafNode*) malloc(sizeof(LeafNode));
  LeafNode* rightLeaf = (LeafNode*) malloc(sizeof(LeafNode));

  // promotionIndex remains in a leaf
  char* promotionKey = splitLeaf(leaf, leftLeaf, rightLeaf);

  printArray(leftLeaf);
  printArray(rightLeaf);

  // promote, split if required, repeat if necessary
  return promotionCycle(root, leaf, leftLeaf, rightLeaf, promotionKey);
}

#define MAX_STRINGS 100
char stringList[MAX_STRINGS][KEY_LENGTH];
int stringCount = 0;
int currentIndex = 0;

void loadStringsFromFile(const char* filename) {
  FILE* file = fopen(filename, "r");
  if (!file) {
    printf("couldn't load file...\n");
    exit(1);
  }

  while (fgets(stringList[stringCount], KEY_LENGTH, file)) {
    stringList[stringCount][strcspn(stringList[stringCount], "\n")] = '\0';
    stringCount++;

    if (stringCount >= MAX_STRINGS) break;
  }
  fclose(file);
}

char* generateString() {
  if (currentIndex >= stringCount) {
    printf("No more strings in list!\n");
    exit(1);
  }
  return stringList[currentIndex++];
}

Entry* makeEntry(int num) {
  Entry* entry = (Entry*) malloc(sizeof(Entry));
  entry->pageNum = num;
  entry->rowOffset = num;
  char* key = generateString();
  strcpy(entry->key, key);
  return entry;
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

int main() {
  loadStringsFromFile("common-words.txt");
  void* root = NULL;

  int numEntries = 100;

  for (int i = 0; i < numEntries; i++) {
    Entry* entry = makeEntry(i);
    printf("string: %s\n", entry->key);
    root = insert(root, entry);
  }

  printTree(root, 0);

  LeafNode* foundLeaf = findLeaf(root, "learn");
  for (int i = 0; i < foundLeaf->numEntries; i++) {
    printf("string: %s pageNum: %d\n", foundLeaf->entries[i].key, foundLeaf->entries[i].pageNum);
  }

  return 0;

}
