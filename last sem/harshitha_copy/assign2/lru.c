#include "fifo.c"

void LRUStrategy(BM_BufferPool *const bm, BM_BufferPoolManager *bmgr){

    void *mgmtData = bm->mgmtData;
    BM_BufferPoolManager *tempBmgr = (BM_BufferPoolManager *) mgmtData;
    int lru_Min = tempBmgr[0].lastAccessedTime;
    int lru_Index = 0;
    SM_FileHandle file;
    int pageFrames = bm->numPages;
    for(int i=0; i< pageFrames; i++){
        if(tempBmgr[i].fixCount == 0){
            lru_Min = tempBmgr[i].lastAccessedTime;
            lru_Index = i;
            break;
        }
    }
    for(int j = lru_Index+1; j<pageFrames; j++){
        if(tempBmgr[j].lastAccessedTime < lru_Min){
            lru_Min = tempBmgr[j].lastAccessedTime;
            lru_Index = j;      
        }
    }
    if(tempBmgr[lru_Index].isDirtyFlag == true){
        char *tempPageFile = bm->pageFile;
        if(openPageFile(tempPageFile, &file) == RC_OK){
            writeBlock(tempBmgr[lru_Index].pageSerial, &file, tempBmgr[lru_Index].data);
            writeIO = writeIO+1;
        }
    }

    frameCount = frameCount + 1;
    bool tmpDirtyFlag = bmgr->isDirtyFlag;
    PageNumber tmpPageSerial = bmgr->pageSerial;
    int tmpFixCount = bmgr->fixCount;
    SM_PageHandle tmpData = bmgr->data;
    tempBmgr[lru_Index].lastAccessedTime = frameCount;
    tempBmgr[lru_Index].isDirtyFlag = tmpDirtyFlag;
    tempBmgr[lru_Index].pageSerial = tmpPageSerial;
    tempBmgr[lru_Index].fixCount = tmpFixCount;
    tempBmgr[lru_Index].data = tmpData;
}