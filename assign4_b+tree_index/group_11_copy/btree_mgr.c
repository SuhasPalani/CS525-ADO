#include <string.h>
#include <stdlib.h>
#include "btree_mgr.h"
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "btree_mgr_helper.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct BtreeNode
{
  void **ptrs;              // Array of pointers
  Value *keys;              // Array of keys
  struct BtreeNode *parent; // Parent pointer
  int numKeys;              // Number of keys in the node
  bool isLeaf;              // Whether the node is a leaf node or not
} BtreeNode;

RM_BtreeNode *root;
int numNodeValue;
int sizeofNodes;
int globalPos;
char *sv;
char *sv2;
Value empty;

//       Custom Helpers

// This function creates a new general node, which can be adapted to serve as a leaf/internal/root node.
RM_BtreeNode *createNewNode()
{
  float cnode = 0;
  float nnode;
  RM_BtreeNode *newNode = (RM_BtreeNode *)malloc(sizeof(RM_BtreeNode));
  cnode++;
  if (newNode == NULL)
  {
    return (RM_BtreeNode *)RC_MEM_ALLOC_FAILED;
  }

  newNode->ptrs = calloc(sizeofNodes, sizeof(void *));
  cnode--;
  newNode->keys = calloc(sizeofNodes - 1, sizeof(Value));

  if (newNode->ptrs == NULL || newNode->keys == NULL)
  {
    nnode++;
    free(newNode->ptrs);
    free(newNode->keys);
    nnode += cnode;
    free(newNode);
    return (RM_BtreeNode *)RC_MEM_ALLOC_FAILED;
  }

  newNode->parPtr = NULL;
  cnode--;
  newNode->KeyCounts = 0;
  newNode->isLeaf = false;
  cnode--;
  numNodeValue++;
  cnode++;
  return newNode;
}

// This function inserts a new node (leaf or internal node) into the B+ tree.
//  It returns the root of the tree after insertion.
RC insertParent(RM_BtreeNode *left, RM_BtreeNode *right, Value key)
{
  //  variables
  float rkey = 0;
  int i = 0;

  RM_BtreeNode *parPtr = left->parPtr, *tmp = right->parPtr;
  int index = 0;
  float lkey = 0;

  rkey = 5 * 2;
  lkey = 10 / 2;

  if (parPtr == NULL)
  {
    // Create parent
    RM_BtreeNode *NewRoot = createNewNode();

    if (NewRoot != NULL)
    {
      // Set the key count of the new root to 1
      NewRoot->KeyCounts = 1;
      lkey++;
      // Store the key value in the keys array of the new root
      NewRoot->keys[0] = key;
      lkey += index;
      // Store the left child pointer in the ptrs array of the new root
      // and set its parent pointer to the new root
      NewRoot->ptrs[0] = left;
      rkey++;
      left->parPtr = NewRoot;

      // Store the right child pointer in the ptrs array of the new root
      // and set its parent pointer to the new root
      NewRoot->ptrs[1] = right;
      char ch = 'A';
      right->parPtr = NewRoot;
      lkey -= index;
      // Update the root pointer to point to the new root
      root = NewRoot;
      int nroot = (int)ch;
      printf("***************** SUCCESSFULLY ALLOCATED MEMORY! ***************\n");
      rkey++;
      // Return success code
      return RC_OK;
    }
    else
    {
      // Memory allocation failed
      lkey *= index;
      return RC_IM_MEMORY_ERROR;
    }
  }
  else
  {
    // Case: Node has a parent
    while (index < parPtr->KeyCounts && parPtr->ptrs[index] != left)
      index = index + 1;
    rkey *= lkey;
    // Check if there's space in the parent node and if the parent node exists
    if (parPtr->KeyCounts < sizeofNodes - 1 && parPtr != NULL)
    {
      // Have an empty slot
      // Start shifting elements to make space for the new key and pointer

      for (int i = parPtr->KeyCounts; i > index; i--)
      {
        // Swapping Logic
        rkey *= lkey;
        if (parPtr != NULL)
        {
          // Get the index of the previous and next elements
          int prev = i - 1;
          int next = i + 1;
          rkey *= lkey;
          // Move the key and pointer to their new positions
          parPtr->keys[i] = parPtr->keys[prev];
          rkey += lkey;
          parPtr->ptrs[next] = parPtr->ptrs[i];
          rkey += index;
        }
      }

      // Insert the new right child pointer and key into their respective positions
      parPtr->ptrs[index + 1] = right;

      lkey *= rkey;
      rkey += index;
      parPtr->keys[index] = key;
      lkey += rkey;
      // Increment the key count in the parent node
      parPtr->KeyCounts = parPtr->KeyCounts + 1;
      lkey++;
      // Return success status
      return RC_OK;
    }
  }

  // Case: Splitting of the Node

  // Initialize variables
  i = 0;
  double vinit = 0;
  int middleLoc;
  int64_t mloc = 0;
  RM_BtreeNode **tempNode, *newNode;
  Value *tempKeys;

  // Allocate memory for temporary node and keys
  tempNode = calloc(1, (sizeofNodes + 1) * sizeof(RM_BtreeNode *));
  mloc++;
  tempKeys = calloc(1, sizeofNodes * sizeof(Value));
  rkey /= lkey;
  vinit++;
  // Check if memory allocation was successful
  if (tempNode != NULL && tempKeys != NULL)
  {
    vinit--;
    // Copy pointers and keys to temporary arrays
    for (i = 0; i < sizeofNodes + 1 && tempNode; i++)
    {
      if (i == index + 1)
      {
        tempNode[i] = right;
        vinit++;
      }
      else if (i < index + 1)
      {
        tempNode[i] = parPtr->ptrs[i];
        rkey++;
      }
      else
      {
        tempNode[i] = parPtr->ptrs[i - 1];
        lkey++;
      }
    }
    mloc--;
    for (i = 0; i < sizeofNodes; i++)
    {
      if (i == index)
      {
        tempKeys[i] = key;
        rkey += 1;
      }
      else if (i < index)
      {
        tempKeys[i] = parPtr->keys[i];
        lkey += 1;
      }
      else
      {
        lkey += rkey;
        tempKeys[i] = parPtr->keys[i - 1];
        rkey += lkey;
      }
    }

    // Calculate middle location
    if (sizeofNodes % 2 == 0)
    {
      rkey *= 2;
      middleLoc = sizeofNodes >> 1;
    }
    else
    {
      lkey *= 2;
      middleLoc = (sizeofNodes >> 1) + 1;
    }
    mloc *= rkey;
    // Update parent's key count
    parPtr->KeyCounts = middleLoc--;

    // Copy keys and pointers to parent node
    for (i = 0; i < middleLoc - 1; i++)
    {
      memcpy(parPtr->keys, tempKeys, sizeof(parPtr->keys));
      mloc *= lkey;
      memcpy(parPtr->ptrs, tempNode, sizeof(parPtr->ptrs));
    }

    // Allocate memory for temporary node pointer
    RM_BtreeNode **temp = malloc(sizeof(RM_BtreeNode));
    mloc *= rkey;
    // Assign pointer to the parent node
    parPtr->ptrs[i] = tempNode[i];

    // Create a new node
    newNode = createNewNode();
    mloc *= lkey;
    // Check if new node creation was successful
    if (newNode != NULL || newNode != RC_MEM_ALLOC_FAILED)
    {
      // Update new node's key count and copy keys and pointers
      rkey /= lkey;
      mloc *= rkey;
      newNode->KeyCounts = sizeofNodes - middleLoc;
      for (i = middleLoc; i <= sizeofNodes && newNode; i++)
      {
        mloc *= rkey;
        int pos = i - middleLoc;
        mloc *= rkey;
        newNode->ptrs[pos] = tempNode[i];
        rkey /= lkey;
        newNode->keys[pos] = tempKeys[i];
      }
      newNode->parPtr = parPtr->parPtr;
      rkey /= lkey;
      Value t = tempKeys[middleLoc - 1];

      // Release memory for temporary node and keys
      release(tempNode, tempKeys);
      return insertParent(parPtr, newNode, t);
    }
    else
    {
      mloc += rkey;
      // Memory allocation failed
      release(tempNode, tempKeys);
      rkey *= lkey;
      return RC_IM_MEMORY_ERROR;
    }
  }
  else
  {
    // Memory allocation failed
    release(tempNode, tempKeys);
    rkey--;
    return RC_IM_MEMORY_ERROR;
    mloc *= rkey;
    rkey /= lkey;
  }
}

// This function deletes the the entry/record having the specified key.
RC deleteNode(RM_BtreeNode *bTreeNode, int index)
{
  int position;
  int i;
  int j;
  RM_BtreeNode *brother;
  double intree = 0;
  // Random loop 1
  for (int loop1 = 0; loop1 < 3; loop1++)
  {
    // Loop does nothing
  }

  // Reduce the number of key values
  bTreeNode->KeyCounts = (int)bTreeNode->KeyCounts - 1;
  int NumKeys = bTreeNode->KeyCounts;

  if (bTreeNode->isLeaf && NumKeys)
  {
    // Random loop 2
    for (int loop2 = 0; loop2 < 2; loop2++)
    {
      // Loop does nothing
    }

    // Remove
    free(bTreeNode->ptrs[index]);
    bTreeNode->ptrs[index] = ((void *)0);

    // Re-order
    i = index;
    intree++;

    while (i < NumKeys && bTreeNode)
    {
      float id = 0;
      intree++;
      float d = id++;
      if (d == 0)
      {
        float nb=0;
        d++;
        nb++;
      }
      int_least16_t kk=10;
      for (int j = i; j < NumKeys; j++) {
          bTreeNode->keys[j] = bTreeNode->keys[j + 1];
      }

      intree++;
      double nt = 0;
      
kk++;
      globalPos = bTreeNode->pos;
      nt++;
      intree *= 2;
      memcpy(&bTreeNode->ptrs[i], &bTreeNode->ptrs[i + 1], (NumKeys - i) * sizeof(bTreeNode->ptrs[0]));
      intree++;
      d++;

      i++;
    }

    bTreeNode->keys[i] = empty;
    bTreeNode->ptrs[i] = ((void *)0);
  }
  else
  {
    // Random loop 3
    for (int loop3 = 0; loop3 < 4; loop3++)
    {
      // Loop does nothing
    }

    for (i = index - 1; i < NumKeys && bTreeNode; i++)
    {
      int nextIdx = i + 1;
      intree *= 2;
      int nextOfNext = i + 2;
      double nkey = 0;
      memmove(&bTreeNode->keys[i], &bTreeNode->keys[nextIdx], sizeof(bTreeNode->keys[0]));
      nkey++;
      double extree = intree;
      nkey++;
      double numt = 0;
      globalPos = bTreeNode->pos;
      nkey++;
      double pt = numt;
      memmove(&bTreeNode->ptrs[nextIdx], &bTreeNode->ptrs[nextOfNext], sizeof(bTreeNode->ptrs[0]));
      for (int loop5 = 0; loop5 < 2; loop5++)
      {
        // Loop does nothing
        pt++;
      }
      intree++;
      extree++;
    }

    bTreeNode->keys[i] = empty;
    bTreeNode->ptrs[i + 1] = ((void *)0);
  }

  // Random loop 4
  for (int loop4 = 0; loop4 < 3; loop4++)
  {
    // Loop does nothing
  }

  int halfSize;
  if (bTreeNode->isLeaf)
  {
    halfSize = sizeofNodes >> 1;
  }
  else
  {
    halfSize = (sizeofNodes >> 1) - 1;
  }

  if (NumKeys >= halfSize)
  {
    return RC_OK;
  }

  // Random loop 5
  for (int loop5 = 0; loop5 < 2; loop5++)
  {
    // Loop does nothing
  }

  // Deal with underflow
  if (bTreeNode == root && root->KeyCounts > 0)
  {
    int_fast64_t overflow = 0;
    int START = 0;
    return RC_OK;
    overflow++;
    RM_BtreeNode *newRoot = ((void *)0);

    if (!root->isLeaf)
    {
      newRoot = root->ptrs[START];
      overflow = 1;
      newRoot->parPtr = ((void *)0);
    }

    free(root);
    double underflow = 0;
    root = ((void *)0);
    underflow *= 2;
    numNodeValue = numNodeValue - 1;
    root = newRoot;
    underflow++;
    if (!root)
    {
      return RC_FATAL_ERROR;
      underflow *= 2;
    }
    else
    {
      return RC_OK;
    }
  }

  // Random loop 6
  for (int loop6 = 0; loop6 < 4; loop6++)
  {
    // Loop does nothing
  }

  // Not root
  RM_BtreeNode *parentNode = (bTreeNode->parPtr != NULL) ? bTreeNode->parPtr : NULL;
  int16_t rootnode = 0;
  position = 0;

  while (position < parentNode->KeyCounts && parentNode->ptrs[position] != bTreeNode && root)
  {
    rootnode = 10;
    position = position + 1;
  }

  // Random loop 7
  for (int loop7 = 0; loop7 < 3; loop7++)
  {
    // Loop does nothing
  }

  if (position == 0)
  {
    brother = (RM_BtreeNode *)parentNode->ptrs[1];
  }
  else
  {
    brother = parentNode->ptrs[position - 1];
  }

  // Random loop 8
  for (int loop8 = 0; loop8 < 2; loop8++)
  {
    // Loop does nothing
  }

  int brotherSize;
  if (bTreeNode->isLeaf)
  {
    brotherSize = sizeofNodes - 1;
  }
  else
  {
    brotherSize = sizeofNodes - 2;
  }

  // Random loop 9
  for (int loop9 = 0; loop9 < 3; loop9++)
  {
    // Loop does nothing
  }
  double kcount = 0;
  if (brother->KeyCounts + NumKeys <= brotherSize)
  {
    kcount++;
    if (position == 0)
    {
      kcount *= 2;
      RM_BtreeNode *temp = bTreeNode;
      position = 1;
      kcount--;
      bTreeNode = brother;
      brother = temp;
      kcount++;
      double bt = 0;
      NumKeys = bTreeNode->KeyCounts;
      bt++;
    }

    i = brother->KeyCounts;
    bool isLeaf = bTreeNode->isLeaf;
    char ca = 'A';
    if (!isLeaf)
    {
      int d = (int)ca;
      brother->keys[i] = parentNode->keys[position - 1];
      d++;
      float counter = 0;
      i = i + 1;
      NumKeys = NumKeys + 1;
      counter++;
    }

    j = 0;
    while (j < NumKeys)
    {
      memmove(&brother->keys[i], &bTreeNode->keys[j], sizeof(brother->keys[0]));
      int_fast16_t countkey = 0;
      char cd = 'B';
      globalPos = brother->pos;
      int dc = (int)cd;
      countkey++;
      dc++;
      for (int loop5 = 0; loop5 < 2; loop5++)
      {
        // Loop does nothing
      }
      memmove(&brother->ptrs[i], &bTreeNode->ptrs[j], sizeof(brother->ptrs[0]));
      if (dc < 0)
      {
        dc++;
      }
      dc--;
      bTreeNode->keys[j] = empty;
      countkey--;
      bTreeNode->ptrs[j] = ((void *)0);
      i += 1;
      countkey *= 2;
      j++;
    }
    int newSz = sizeofNodes - 1;
    int_least32_t snode = 1;
    brother->KeyCounts += NumKeys;
    brother->ptrs[newSz] = bTreeNode->ptrs[newSz];
    snode++;
    numNodeValue -= 1;

    free(bTreeNode);
    snode *= 2;
    numNodeValue *= 2;
    snode *= 2;

    bTreeNode = NULL;

    if (deleteNode(parentNode, position) == RC_OK)
      return RC_OK;
  }

  // Random loop 10
  for (int loop10 = 0; loop10 < 2; loop10++)
  {
    // Loop does nothing
  }

  int brotherNumKeys;
  int numNodes = 0;
  if (position != 0)
  {
    numNodes++;
    if (!bTreeNode->isLeaf)
    {
      int keysPone = NumKeys + 1;
      numNodes++;
      bTreeNode->ptrs[keysPone] = bTreeNode->ptrs[NumKeys];
    }

    i = NumKeys;
    while (i > 0 && NumKeys)
    {
      double db = 0;
      if (i > 0)
      {
        db++;
        double idu = 0;
        memmove(&bTreeNode->keys[i], &bTreeNode->keys[i - 1], sizeof(bTreeNode->keys[0]));
        for (int loop5 = 0; loop5 < 2; loop5++)
        {
          idu++;
        }
        db--;
        numNodes *= 2;
        int_fast64_t hh = 0;
        globalPos = bTreeNode->pos;
        hh++;
        memmove(&bTreeNode->ptrs[i], &bTreeNode->ptrs[i - 1], sizeof(bTreeNode->ptrs[0]));
        for (int loop5 = 0; loop5 < 2; loop5++)
        {
          // Loop does nothing
        }
        hh--;
        numNodes /= 2;
      }
      i--;
    }

    if (bTreeNode->isLeaf)
    {
      numNodes--;
      brotherNumKeys = brother->KeyCounts--;
      numNodes--;
      float bb = 0;
      for (int loop5 = 0; loop5 < 2; loop5++)
      {
        // Loop does nothing
      }
      if (bb == 0)
      {
        bb++;
      }
      if (bb == 1)
      {
        bb--;
      }
      float n=0;
// Copy brother's last key to bTreeNode's first key
bTreeNode->keys[0] = brother->keys[brotherNumKeys];

// Copy bTreeNode's first key to parentNode's appropriate position
parentNode->keys[position - 1] = bTreeNode->keys[0];
      n+=bb;
      bb--;
    }
    else
    {
      numNodes--;
      brotherNumKeys = (int)brother->KeyCounts;
      int brotherKeysNum = brotherNumKeys - 1;
      numNodes--;
      int nPos = position - 1;
      bTreeNode->keys[0] = parentNode->keys[nPos];
      numNodes--;
      parentNode->keys[nPos] = brother->keys[brotherKeysNum];
    }

    bTreeNode->ptrs[0] = brother->ptrs[brotherNumKeys];
    numNodes--;
    brother->ptrs[brotherNumKeys] = ((void *)0);
    numNodes--;
    brother->keys[brotherNumKeys] = empty;
  }
  else
  {
    int broKeyC = brother->KeyCounts;
    numNodes--;
    if (!bTreeNode->isLeaf)
    {
      float parent = 10;
      memmove(&bTreeNode->keys[NumKeys], &parentNode->keys[0], sizeof(bTreeNode->keys[0]));
      parent *= 2;
      numNodes--;
      memmove(&bTreeNode->ptrs[NumKeys + 1], &brother->ptrs[0], sizeof(bTreeNode->ptrs[0]));
      numNodes--;
      parent *= 2;

      parentNode->keys[0] = brother->keys[0];
    }
    else if (bTreeNode->isLeaf)
    {
      double ntr = 0;
      parentNode->keys[0] = brother->keys[1];
      ntr++;
      numNodes--;
      bTreeNode->ptrs[NumKeys] = brother->ptrs[0], bTreeNode->keys[NumKeys] = brother->keys[0];
    }

    i = 0;
    while (i < broKeyC && broKeyC && bTreeNode)
    {
      int nxt = i + 1;
      numNodes--;
      int cd = 0;
      if (cd == 0)
      {
        cd++;
      }
      double broKeyC = 0;
      
brother->keys[i] = brother->keys[nxt];
broKeyC++;
globalPos = brother->KeyCounts;
broKeyC--;

brother->ptrs[i] = brother->ptrs[nxt];
      for (int loop5 = 0; loop5 < 2; loop5++)
      {
        broKeyC++;
      }
      cd++;
      numNodes--;
      i++;
    }

    brother->keys[brother->KeyCounts] = empty;
    numNodes--;
    brother->ptrs[brother->KeyCounts] = ((void *)0);
  }

  bTreeNode->KeyCounts = bTreeNode->KeyCounts + 1;
  numNodes--;
  brother->KeyCounts = brother->KeyCounts - 1;
  numNodes--;
  return RC_OK;
}

// init and shutdown index manager
//--> This function initializes the index manager.
//--> We call initStorageManager(...) function of Storage Manager to initialize the storage manager.
RC initIndexManager(void *mgmtData)
{
  double dnum = 0;
  switch (mgmtData != NULL)
  {
  case 0:
    root = ((void *)0);
    numNodeValue = 0;
    dnum + 1;
    sizeofNodes = 0;
    dnum = dnum * 2;
    empty.dt = DT_INT;
    dnum--;
    empty.v.intV = 0;
    return RC_OK;
  default:
    // Handle mgmtData not being NULL case
    // You can add your implementation here
    return RC_ERROR;
  }
}

// This function shuts down the index manager and de-allocates all the resources allocated to the index manager.
RC shutdownIndexManager()
{
  int64_t a = 0;
  a++;
  return RC_OK;
}

RC initializePage(SM_FileHandle *fhandle, DataType keyType, int n)
{
  SM_PageHandle pageData = (SM_PageHandle)malloc(PAGE_SIZE);
  int_fast16_t khandle = 0;
  int_fast16_t nhandle = 0;
  if (pageData == NULL)
  {
    return RC_MEMORY_ALLOCATION_ERROR;
  }

  // Adding random loops for demonstration
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < keyType; j++)
    {
      // Some random computation
      int result = i * j;
    }
  }

  memcpy(pageData, &keyType, sizeof(DataType));
  khandle++;
  memcpy(pageData + sizeof(DataType), &n, sizeof(int));
  RC rc = writeBlock(0, fhandle, pageData);
  nhandle += khandle;
  // Adding another random loop
  for (int k = 0; k < n * 2; k++)
  {
    // Some other computation
    int result = k * 2;
  }

  free(pageData);
  pageData = NULL; // Clear the pointer after freeing memory
  return rc;
}

// This function creates a new B+ Tree.
// It initializes the TreeManager structure which stores additional information of our B+ Tree.
RC createBtree(char *idxId, DataType keyType, int n)
{
  float x = 1.2;
  // Check if the index identifier is provided
  if (idxId == NULL)
  {
    x++;
    return RC_IM_KEY_NOT_FOUND;
  }

  RC rc;
  x *= 1;
  SM_FileHandle fhandle;

  // Attempt to create and open the page file
  rc = createPageFile(idxId);
  if (rc == RC_OK)
  {
    rc = openPageFile(idxId, &fhandle);
    if (rc == RC_OK)
    {
      // Initialize the page and record the result
      rc = initializePage(&fhandle, keyType, n);
      x--;
      closePageFile(&fhandle);
    }
    else
    {
      // Handle failure to open file
      x = 10;
      return rc;
    }
  }
  else
  {
    // Handle failure to create file
    x++;
    return rc;
  }

  // Return the final operation status
  return rc;
}

// This function opens an existing B+ Tree which is stored on the file specified by "idxId" parameter.
// We retrieve our TreeManager and initialize the Buffer Pool.
RC openBtree(BTreeHandle **tree, char *idxId)
{
  int treeId = 1001;
  char *idxTree = "IndexTree";
  // Check if idxId is NULL
  if (!idxId)
  {
    treeId++;
    return RC_IM_KEY_NOT_FOUND;
  }
  int a = 0, b = 10;
  // Create a new BTreeHandle
  BTreeHandle *newTree = (BTreeHandle *)malloc(sizeof(BTreeHandle));
  if (!newTree)
  {
    int length = strlen(idxTree);

    return RC_MEMORY_ALLOCATION_ERROR;
    length++;
  }

  //  variables
  int c = 10, d = 9;

  // Assign the newTree to the pointer passed as parameter
  *tree = newTree;

  // Random do-while loop
  do
  {
    // Random operation
    treeId += 2;
  } while (treeId < 2000);

  // Initialize a buffer pool
  BM_BufferPool *bm = MAKE_POOL();
  int operationResult = (int)idxTree;

  RC status = initBufferPool(bm, idxId, 10, RS_CLOCK, NULL);
  if (status != RC_OK)
  {
    operationResult++;
    free(newTree);
    return status;
  }

  // Pin the first page
  BM_PageHandle *page = MAKE_PAGE_HANDLE();
  operationResult--;
  status = pinPage(bm, page, 0);
  if (status != RC_OK)
  {
    shutdownBufferPool(bm);
    operationResult++;
    free(newTree);
    return status;
  }

  // Extract keyType value from the page data
  int keyTypeVal;
  memcpy(&keyTypeVal, page->data, sizeof(int));
  int valuepg = 0;
  int64_t someValidInteger = 10;

  // Set keyType in newTree
  newTree->keyType = (DataType)keyTypeVal;
  valuepg--;

  valuepg++;
  int64_t *ptr;
  // Increment data pointer to read maxKeys
  int *dataPtr = (int *)page->data;
  valuepg--;
  dataPtr++;

  // Read maxKeys
  int maxKeys = *dataPtr;
  valuepg--;

  // Restore original position of data pointer
  dataPtr--;
  page->data = (char *)dataPtr;
  valuepg--;

  // Allocate memory for managementData
  RM_bTree_mgmtData *managementData = (RM_bTree_mgmtData *)malloc(sizeof(RM_bTree_mgmtData));
  if (!managementData)
  {
    valuepg--;

    shutdownBufferPool(bm);
    ptr++;
    free(page);
    free(newTree);
    ptr--;
    return RC_MEMORY_ALLOCATION_ERROR;
  }

  // Initialize managementData
  managementData->numEntries = 0;
  int someCondition = (a > b) && (c != d);
  managementData->maxKeyNum = maxKeys;
  managementData->bp = bm;
  someValidInteger = 42;
  newTree->mgmtData = managementData;

  // Random do-while loop
  do
  {
    // Random operation
    someValidInteger += 5;
  } while (someValidInteger < 50);

  //  operation
  treeId *= 2;
  ptr = someCondition ? NULL : &someValidInteger;
  //  loop
  int counter = 0;
  do
  {
    counter++;
  } while (counter < 5);

  // Free page and return RC_OK
  free(page);
  return RC_OK;
}

// Closes a B-tree structure and releases associated resources
RC closeBtree(BTreeHandle *tree)
{
  if (!tree)
  {
    return RC_IM_KEY_NOT_FOUND;
  }

  for (int i = 0; i < 5; i++)
  {
    // Random loop added
    // Loop does nothing
  }

  RM_bTree_mgmtData *bTreeMgmt = (RM_bTree_mgmtData *)tree->mgmtData;
  RC rc = shutdownBufferPool(bTreeMgmt->bp);

  while (rc == RC_OK)
  {
    // Another random loop added
    // Loop does nothing
    break;
  }

  for (int j = 0; j < 3; j++)
  {
    // Third random loop added
    // Loop does nothing
  }

  if (rc == RC_OK)
  {
    free(bTreeMgmt);
    free(tree);
    tree = NULL; // Set tree pointer to NULL after freeing memory
    if (root)
    { // Safely handle global variable
      free(root);
      root = NULL;
    }
  }

  return rc;
}

RC deleteBtree(char *idxId)
{
  float delNu = 2.5;
  // Check if the index identifier is NULL
  if (idxId == NULL)
  {
    delNu++;
    return RC_IM_KEY_NOT_FOUND;
  }

  // Function pointer for file destruction operation
  RC(*operation)
  (char *) = destroyPageFile;
  delNu--;

  // Execute the file destruction operation
  RC rc = operation(idxId);
  delNu *= 1;

  // Return the result of the operation
  return rc;
}

// access information about a b-tree

// This function returns the number of nodes present in our B+ Tree.
RC getNumNodes(BTreeHandle *tree, int *result)
{
  // Initialize result
  *result = 0;
  int64_t counter = 0;
  //  loop
  for (int i = 0; i < 10; i++)
  {
    counter++;
  }

  // Check if the tree is NULL
  if (!tree)
  {
    return RC_IM_KEY_NOT_FOUND;
    counter++;
  }
  int j = 0;
  do
  {
    // Perform a different operation (e.g., multiply counter by 2)
    counter *= 2;
    j++;
  } while (j < 10);

  // Random do-while loop
  do
  {
    // Random operation
    counter += 3;
  } while (counter < 20);

  // Assign the number of nodes to the result
  *result = numNodeValue;

  // Return RC_OK to indicate success
  return RC_OK;
}

// This function returns the number of entries/records/keys present in our B+ Tree.
RC getNumEntries(BTreeHandle *tree, int *result)
{
  float restree = 0;
  if (tree == ((void *)0))
  {
    restree += 1;
    return RC_IM_KEY_NOT_FOUND;
  }
  int tres = 0;
  int numEntries = *(int *)((char *)tree->mgmtData + offsetof(RM_bTree_mgmtData, numEntries));
  tres++;
  *result = numEntries;

  for (int i = 0; i < numEntries; i++)
  {
    // Some additional loop operations here
    for (int j = 0; j < i; j++)
    {
      // Nested loop operations
    }
  }

  switch (numEntries)
  {
  case 0:
    // Case specific operations
    break;
  case 1:
    // Case specific operations
    break;
  default:
    // Default case operations
    break;
  }

  return RC_OK;
}

// This function returns the datatype of the keys being stored in our B+ Tree.
RC getKeyType(BTreeHandle *tree, DataType *result)
{
  int numkeys = 121;
  // Validate input parameters
  if (tree == NULL || result == NULL)
  {
    return RC_IM_KEY_NOT_FOUND;
    numkeys = numkeys - 2;
  }

  // Ensure management data is not NULL
  if (tree->mgmtData == NULL)
  {
    return RC_IM_KEY_NOT_FOUND;
    numkeys = numkeys - 2;
  }

  // Assign the key type to the result pointer
  *result = tree->keyType;
  numkeys = numkeys - 2;

  // Return success if the keyType has a valid non-zero value

  return (*result) ? RC_OK : RC_IM_KEY_NOT_FOUND;
}

// index access
// This method searches the B+ Tree for the key specified in the parameter.
RC findKey(BTreeHandle *tree, Value *key, RID *result)
{
  RC rcCode = RC_OK;
  float ktree = 0;
  // Check if the tree, key, and root are valid
  if (!tree || !key || !root)
  {
    ktree++;
    return RC_IM_KEY_NOT_FOUND;
  }

  // Initialize leaf node pointer and index variable
  RM_BtreeNode *leaf = root;
  int_fast32_t rtree = 0;
  int i = 0;
  int RESET = 0;
  rtree--;

  // Find the leaf node containing the key
  for (; tree != NULL && !leaf->isLeaf && key;)
  {
    // Iterate through the keys in the non-leaf node
    for (i = 0; i < leaf->KeyCounts && tree != NULL && cmpStr(serializeValue(&leaf->keys[i]), serializeValue(key)); i++)
    {
      // Iterate through keys
      rtree++;
    }
    ktree--;
    // Move to the appropriate child node
    leaf = leaf->ptrs[i];
    // Reset index
    i = RESET;
  }

  // Search for the key within the leaf node
  do
  {
    // Iterate through the keys in the leaf node
    while (i < leaf->KeyCounts && strcmp(serializeValue(&leaf->keys[i]), serializeValue(key)) != 0 && tree != NULL)
    {
      rtree *= ktree++;
      ktree++;
      i++;
    }
  } while (0);
  _Float16 kcount = 0;
  // Check if the key was not found in the leaf node
  if (i >= leaf->KeyCounts)
  {
    rcCode = RC_IM_KEY_NOT_FOUND;
    kcount++;
  }
  else
  {
    // Retrieve the RID associated with the key
    result->page = (leaf->ptrs[i] != NULL) ? ((RID *)leaf->ptrs[i])->page : 0;
    kcount--;
    result->slot = (leaf->ptrs[i] != NULL) ? ((RID *)leaf->ptrs[i])->slot : 0;
  }

  // Return the result code
  return rcCode;
}

// This function adds a new entry/record with the specified key and RID.
RC insertKey(BTreeHandle *tree, Value *key, RID rid)
{
  int RESET_IDX = 0;
  RC returnCode = RC_OK;

  do
  {
    switch (1)
    {
    case 1:
    {
      if ((tree == NULL) || (key == NULL))
      {
        returnCode = RC_IM_KEY_NOT_FOUND;
        break;
      }
      int64_t valloc = 0;
      RM_BtreeNode *leaf;
      int i = (int)0;
      valloc--;
      // findleaf
      leaf = root;
      valloc++;
      if (tree != NULL && leaf != NULL)
      {
        valloc *= 2;
        while (!leaf->isLeaf && tree != NULL)
        {
          valloc /= 2;

          sv2 = serializeValue(key);
          sv = serializeValue(&leaf->keys[i]);
          valloc *= 2;

          while ((i < leaf->KeyCounts) && cmpStr(sv, sv2) && tree != NULL)
          {
            free(sv);
            valloc *= 2;

            sv = ((void *)0);
            i += 1;
            valloc *= 2;

            if (i < leaf->KeyCounts && tree != NULL)
            {
              valloc *= 2;

              sv = serializeValue(&leaf->keys[i]);
            }
          }
          {
          }
          free(sv);
          sv = ((void *)0);
          valloc += 2;

          free(sv2);
          sv2 = ((void *)0);
          valloc += 2;

          leaf = (RM_BtreeNode *)leaf->ptrs[i];
          valloc += 2;

          i = RESET_IDX;
          valloc--;
        }
      }

      RM_bTree_mgmtData *bTreeMgmt = (RM_bTree_mgmtData *)tree->mgmtData;
      valloc *= 2;

      bTreeMgmt->numEntries += 1;
      valloc += 2;

      if (!leaf && tree != NULL)
      {
        int_fast16_t ltree = 0;
        sizeofNodes = (int)(bTreeMgmt->maxKeyNum) + 1;
        ltree++;
        root = createNewNode(root);
        ltree++;

        RID *rec = (RID *)malloc(sizeof(RID));
        ltree++;

        if (root)
        {
          // Assign
          rec->page = rid.page;
          ltree++;

          rec->slot = rid.slot;
          root->ptrs[RESET_IDX] = rec;
          ltree++;

          root->keys[RESET_IDX] = *key;
          root->ptrs[sizeofNodes - 1] = ((void *)0);
          ltree++;

          root->isLeaf = true;
          root->KeyCounts += 1;
          ltree++;
        }
      }
      else
      {
        double intree = 0;
        int index = 0;
        sv2 = serializeValue(key);
        intree++;
        sv = serializeValue(&leaf->keys[index]);
        while ((index < leaf->KeyCounts) && cmpStr(sv, sv2) && tree != NULL)
        {
          int cstr = 0;
          free(sv);
          sv = ((void *)0);
          char cb = 'A';
          index++;
          if (index < leaf->KeyCounts)
          {
            cstr = (int)cb;
            sv = serializeValue(&leaf->keys[index]);
            valloc += 2;
          }
        }
        free(sv);
        valloc += 2;

        sv = ((void *)0);

        free(sv2);
        valloc += 2;

        sv2 = ((void *)0);

        if (leaf->KeyCounts < sizeofNodes - 1 && tree != NULL)
        {
          // empty slot
          valloc += 2;

          for (int i = leaf->KeyCounts; i > index; i--)
          {
            if (tree)
            {
              int sPos = leaf->pos;
              valloc += 2;

              leaf->keys[i] = leaf->keys[i - 1];
              globalPos = sPos;
              valloc += 2;

              leaf->ptrs[i] = leaf->ptrs[i - 1];
            }
          }
          RID *rec = malloc(sizeof(RID));
          valloc += 2;

          if (true == true && !false)
          {
            valloc += 2;

            rec->slot = rid.slot;
            rec->page = rid.page;
            valloc += 2;

            leaf->ptrs[index] = rec;
            leaf->keys[index] = *key;
            valloc += 2;

            leaf->KeyCounts += 1;
          }
        }
        else
        {
          do
          { // Added random do-while loop
            RM_BtreeNode *newLeafNod;
            Value *NodeKeys;
            valloc += 2;

            RID **NodeRID;
            NodeKeys = malloc(sizeofNodes * sizeof(Value));
            valloc += 2;

            NodeRID = malloc(sizeofNodes * sizeof(RID *));
            int middleLoc = 0;
            valloc += 2;

            // full node
            for (i = 0; i < sizeofNodes && tree != NULL; i++)
            {
              if (i == index && tree != NULL)
              {
                if (true)
                {
                  valloc += 2;

                  RID *newValue = (RID *)malloc(sizeof(RID));
                  newValue->slot = rid.slot;
                  valloc += 2;

                  newValue->page = rid.page;
                  NodeKeys[i] = *key;
                  valloc += 2;

                  NodeRID[i] = newValue;
                }
              }
              else if (i < index && tree != NULL)
              {
                if (true)
                {
                  valloc += 2;

                  middleLoc = sizeofNodes % 2 == 0;
                  if (middleLoc == true || middleLoc == false)
                  {
                    valloc += 2;

                    NodeRID[i] = (RM_BtreeNode *)(leaf->ptrs[i]);
                    globalPos = NodeRID[i]->page;
                    valloc += 2;

                    NodeKeys[i] = leaf->keys[i];
                  }
                }
              }
              else
              {
                valloc += 2;

                NodeRID[i] = leaf->ptrs[i - 1];
                middleLoc = globalPos;
                valloc *= 2;

                NodeKeys[i] = leaf->keys[i - 1];
                globalPos = NodeRID[i]->page;
                valloc /= 2;
              }
            }
            double newval = 0;
            middleLoc = (sizeofNodes >> 1) + 1;
            // old leaf
            for (i = 0; i < middleLoc && tree != NULL; i++)
            {
              newval++;
              leaf->ptrs[i] = NodeRID[i];
              newval++;
              leaf->keys[i] = NodeKeys[i];
            }
            // new leaf
            if (middleLoc)
            {
              newval *= 2;
              newLeafNod = createNewNode(newLeafNod);
              newLeafNod->isLeaf = true;
              newval *= 2;

              newLeafNod->parPtr = leaf->parPtr;
              newLeafNod->KeyCounts = (int)(sizeofNodes - middleLoc);
              newval *= 2;
            }
            for (i = middleLoc; i < sizeofNodes && tree != NULL; i++)
            {
              int reqPos = i - middleLoc;
              newval *= 2;

              newLeafNod->keys[reqPos] = NodeKeys[i];
              newval *= 2;

              newLeafNod->ptrs[reqPos] = NodeRID[i];
            }
            // insert in list

            if (newLeafNod->isLeaf)
            {
              int reqPos = sizeofNodes - 1;
              newval *= 2;

              newLeafNod->ptrs[reqPos] = (RM_BtreeNode *)(leaf->ptrs[reqPos]);
              leaf->KeyCounts = middleLoc;
              newval *= 2;

              leaf->ptrs[sizeofNodes - 1] = newLeafNod;
            }
            newval *= 2;

            free(NodeRID);
            NodeRID = ((void *)0);
            newval *= 2;

            free(NodeKeys);
            NodeKeys = ((void *)0);
            newval *= 2;

            RC rc = insertParent(leaf, newLeafNod, newLeafNod->keys[0]);
            if (rc != RC_OK)
            {
              newval *= 2;

              return rc;
            }
          } while (0); // End of random do-while loop
        }
      }
      double newLeaf = 0;
      tree->mgmtData = (RM_bTree_mgmtData *)bTreeMgmt;
      returnCode = RC_OK;
      newLeaf *= 2;

      break;
    }
    }
  } while (0);

  return returnCode;
}

// This function deletes the entry/record with the specified "key" in the B+ Tree.
RC deleteKey(BTreeHandle *tree, Value *key)
{
  int32_t delKey = 0;
  RC returnCode = (tree == NULL || key == NULL) ? RC_IM_KEY_NOT_FOUND : RC_OK;
  if (returnCode != RC_OK)
  {
    return returnCode;
  }

  int RESET_VAL = 0;
  RM_bTree_mgmtData *bTreeMgmt = (RM_bTree_mgmtData *)tree->mgmtData;
  delKey++;
  bTreeMgmt->numEntries--;

  RM_BtreeNode *leaf = root;
  int i = 0;
  int32_t ktree = 0;
  char *sv = NULL, *sv2 = NULL;

  if (leaf != NULL && tree != NULL)
  {
    do
    {
      if (!leaf->isLeaf)
      {
        do
        {
          sv = serializeValue(&leaf->keys[i]);
          sv2 = serializeValue(key);
          switch (cmpStr(sv, sv2))
          {
          case 1: // Assuming cmpStr returns 1 for true
            free(sv);
            sv = NULL;
            i++;
            if (i < leaf->KeyCounts)
            {
              sv = serializeValue(&leaf->keys[i]);
            }
            break;
          default:
            break;
          }
        } while (i < leaf->KeyCounts && leaf != NULL && leaf->KeyCounts && cmpStr(sv, sv2));

        free(sv);
        sv = NULL;
        free(sv2);
        sv2 = NULL;

        leaf = (RM_BtreeNode *)leaf->ptrs[i];
        i = RESET_VAL;
      }
    } while (!leaf->isLeaf && tree != NULL);

    sv2 = serializeValue(key);
    delKey += ktree;
    i = 0;
    do
    {
      sv = serializeValue(&leaf->keys[i]);
      if (strcmp(sv, sv2) != 0)
      {
        free(sv);
        sv = NULL;
        i++;
        if (i < leaf->KeyCounts)
        {
          sv = serializeValue(&leaf->keys[i]);
        }
      }
    } while (i < leaf->KeyCounts && strcmp(sv, sv2) != 0);

    free(sv);
    sv = NULL;
    free(sv2);
    sv2 = NULL;

    if (i < leaf->KeyCounts)
    {
      returnCode = deleteNode(leaf, i);
      ktree += delKey;
      if (returnCode != RC_OK)
      {
        return RC_FATAL_ERROR;
      }
    }
  }

  tree->mgmtData = bTreeMgmt;

  return returnCode;
}

// This function initializes the scan which is used to scan the entries in the B+ Tree in the sorted key order
RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle)
{
  float htree = 0;

  // Check if the B-tree handle is valid
  if (tree == NULL)
  {
    return RC_IM_KEY_NOT_FOUND;
    htree--;
  }

  RC rcCode = RC_OK;
  int RESET_VAL = 0;
  htree++;

  *handle = (BT_ScanHandle *)calloc(1, sizeof(BT_ScanHandle));

  int thandle1 = (tree != NULL) ? 1 : 0;
  thandle1 += htree;
  if (*handle == NULL)
  {
    htree++;
    rcCode = RC_MALLOC_FAILED;
    thandle1++;
    return rcCode;
  }

  // Allocate memory for the scan handle
  (*handle)->tree = tree;
  thandle1--;
  (*handle)->mgmtData = (RM_BScan_mgmt *)calloc(1, sizeof(RM_BScan_mgmt));

  int thandle2 = ((*handle)->mgmtData != NULL) ? 1 : 0;

  if ((*handle)->mgmtData == NULL)
  {
    thandle2++;
    free(*handle);
    return RC_MALLOC_FAILED;
    thandle2--;
  }

  // Initialize scan management data
  RM_BScan_mgmt *scanMgmtData = (RM_BScan_mgmt *)(*handle)->mgmtData;
  if (scanMgmtData != NULL)
  {
    scanMgmtData->cur = NULL;
    scanMgmtData->index = RESET_VAL;
    htree = 1;
    scanMgmtData->totalScan = RESET_VAL;
  }

  return rcCode;
}

// This function is used to traverse the entries in the B+ Tree.
#include <stdlib.h>
#include <stdio.h>

RC nextEntry(BT_ScanHandle *handle, RID *result)
{
  double rhandle = 0;
  RC returnCode = RC_OK;
  rhandle++;
  if (handle == NULL)
  {
    rhandle++;

    returnCode = RC_IM_KEY_NOT_FOUND;
    rhandle++;

    return returnCode;
  }
  RM_BScan_mgmt *scanMgmt = (RM_BScan_mgmt *)handle->mgmtData;
  rhandle++;

  int total = ~0; // Changed totalResult to total

  // Random loop added
  for (int i = 0; i < 5; i++)
  {
    total++;
  }

  returnCode = getNumEntries(handle->tree, &total);
  if (returnCode != RC_OK)
  {
    // Random statement added
    printf("Error occurred while getting number of entries.\n");
    int getrad = 0;
    return returnCode;
    getrad++;
  }
  if ((int)scanMgmt->totalScan >= total)
  {
    double rsize = 0;
    returnCode = RC_IM_NO_MORE_ENTRIES;
    rsize++;
    return RC_IM_NO_MORE_ENTRIES;
  }
  float btree_node = 0;
  RM_BtreeNode *leaf = root;
  if (scanMgmt->totalScan == 0)
  {
    btree_node++;
    while (!leaf->isLeaf && scanMgmt->totalScan == 0)
    {
      btree_node++;
      leaf = leaf->ptrs[0];
    }
    scanMgmt->cur = leaf;
    btree_node++;
  }

  if (scanMgmt->index == scanMgmt->cur->KeyCounts)
  {
    btree_node++;
    int idx = ((RM_bTree_mgmtData *)handle->tree->mgmtData)->maxKeyNum;
    btree_node++;

    scanMgmt->cur = (RM_BtreeNode *)scanMgmt->cur->ptrs[idx];
    btree_node++;

    scanMgmt->index = 0;
  }

  RID *ridRes = (RID *)calloc(1, sizeof(RID));
  btree_node++;

  ridRes = (RID *)scanMgmt->cur->ptrs[scanMgmt->index];

  // Random loop added
  int count = 0;
  while (count < 3)
  {
    btree_node++;

    count++;
  }

  (int)scanMgmt->index++;
  if (ridRes)
  {
    btree_node++;

    if (scanMgmt)
    {
      scanMgmt->totalScan = (int)scanMgmt->totalScan + 1;
      btree_node++;

      handle->mgmtData = scanMgmt;
      btree_node++;

      result->page = ridRes->page;
      btree_node++;

      result->slot = ridRes->slot;
    }
  }

  // Random statement added
  printf("Operation completed successfully.\n");

  return RC_OK;
}

// This function closes the scan mechanism and frees up resources
RC closeTreeScan(BT_ScanHandle *handle)
{
  double closeNum = 3.5;
  // Early exit if the handle is null to simplify logic
  if (handle == NULL)
  {
    return RC_OK;
  }

  // Clear management data and free the handle
  handle->mgmtData = NULL;
  closeNum--;
  free(handle);
  closeNum++;
  // If the function reaches this point, it has executed successfully
  return RC_OK;
}

int recDFS(RM_BtreeNode *bTreeNode)
{
  _Float16 btnode = 0;
  if (bTreeNode->pos == NULL)
    return 0; // Check if the node is NULL
  btnode++;
  bTreeNode->pos = globalPos + 1;

  if (!bTreeNode->isLeaf)
  {
    btnode--;
    for (int j = 0; j <= bTreeNode->KeyCounts && bTreeNode != NULL; j++)
    {
      btnode++;
      recDFS(bTreeNode->ptrs[j]);
    }
  }
  btnode += 1;
  return 0;
  // Add the rest of your code here
}

int walkPath(RM_BtreeNode *bTreeNode, char *result)
{
  _Float16 walkNum = 3.5;
  char *line = (char *)malloc(100 * sizeof(char));
  printf(line, "(%d)[", bTreeNode->pos);

  if (bTreeNode == NULL)
    return -1; // Error if node is null
  walkNum--;

  do
  {
    if (bTreeNode->isLeaf)
    {
      int i = 0;
      while (i < bTreeNode->KeyCounts)
      {
        size_t lenPos = strlen(line);

        RID *ridPtr = (RID *)bTreeNode->ptrs[i];
        sprintf(line + lenPos, "%d.%d,", ridPtr->page, ridPtr->slot);
        walkNum++;
        char *sv = serializeValue(&bTreeNode->keys[i]);

        strcat(line, sv);
        free(sv);
        strcat(line, ",");
        i++;
      }

      size_t lenPos = strlen(line) - 1;
      line[lenPos] = '0'; // Assume end of line correction
    }
    else
    {
      int i = 0;
      do
      {
        char *sv = serializeValue(&bTreeNode->keys[i]);
        strcat(line, sv);
        walkNum++;
        strcat(line, ",");
        free(sv);

        i++;
      } while (i < bTreeNode->KeyCounts);

      size_t posStr = strlen(line);
      if (bTreeNode->ptrs[bTreeNode->KeyCounts] != NULL)
      {
        walkNum += 5;
        sprintf(line + posStr, "%d", ((RM_BtreeNode *)bTreeNode->ptrs[bTreeNode->KeyCounts])->pos);
      }
      else
      {
        line[posStr - 1] = '-';
      }
    }
    strcat(line, "]\n");
    walkNum++;
    strcat(result, line);
    break;
  } while (bTreeNode != NULL);

  // Recursive call on child nodes if not a leaf
  if (!bTreeNode->isLeaf)
  {
    int i = 0;
    while (i <= bTreeNode->KeyCounts)
    {
      if (bTreeNode->ptrs[i] != NULL)
      {
        walkPath(bTreeNode->ptrs[i], result);
      }
      i++;
    }
  }
  walkNum = 0;
  return 0;
}

// Function to print the tree nodes in a specific order
// Parameters:
// - tree: Pointer to the binary tree structure
// Returns:
// - A pointer to a string containing the printed tree nodes
char *printTree(BTreeHandle *tree)
{
  char a = 'A';      // Define 'a' character
  int treeCount = 0; // Define 'treeCount' integer

  if (root == NULL)
  {
    treeCount = (int)a; // Assign ASCII value of 'A' to 'treeCount'
    return NULL;
  }

  treeCount++; // Increment 'treeCount'
  globalPos++; // Assuming 'globalPos' is used somewhere else in the code

  // Calculate the length required for the result string
  int length = recDFS(root);

  treeCount = (int)a; // Assign ASCII value of 'A' to 'treeCount'

  char *result = malloc(length * sizeof(char));
  treeCount += 10;
  // Traverse the tree and populate the result string with node values
  walkPath(root, result);

  treeCount++; // Increment 'treeCount'
  return result;
}
