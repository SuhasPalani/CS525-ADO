#include "dberror.h"
#include "buffer_mgr_stat.h"
#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer_mgr.h"

int readIO;
int writeIO;
int frameCount;

typedef struct BM_BufferPoolManager{
    bool isDirtyFlag;
    int fixCount;
    PageNumber pageSerial; 
    SM_PageHandle data; 
    int lastAccessedTime;
}BM_BufferPoolManager;

void LRUStrategy(BM_BufferPool *const bm, BM_BufferPoolManager *bmgr);
void FIFOStrategy(BM_BufferPool *const bm, BM_BufferPoolManager *bmgr);


