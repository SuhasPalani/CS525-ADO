#include "lru.c"

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"buffer_mgr.h"
#include"storage_mgr.h"
#include"dberror.h"

int readIO, writeIO, frameCount;

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData){
    
    if(pageFileName != NULL){ 
        if(numPages > 0){
              readIO = 0;
    writeIO = 0;
    frameCount = 0;
            BM_BufferPoolManager *bmgr = malloc(sizeof(BM_BufferPoolManager) * numPages);
            if( bmgr != NULL){
                    int pageIndex = 0;
                    bool isDirtyFlag = FALSE;
                    int fixCount = 0;
                    PageNumber pageSerial = NO_PAGE;
                    int lastAccessedTime = 0;
                    while (pageIndex < numPages) {
                        
                        bmgr[pageIndex] = (BM_BufferPoolManager){
                            .isDirtyFlag = isDirtyFlag,
                            .fixCount = fixCount,
                            .pageSerial = pageSerial,
                            .data = NULL,
                            .lastAccessedTime = lastAccessedTime
                        };
                        pageIndex++;
                    }

                    *bm = (BM_BufferPool) {
                        .mgmtData = bmgr,
                        .numPages = numPages,
                        .pageFile = (char *)pageFileName,
                        .strategy = strategy
                    };
                return RC_OK;

            }
            else{
                return RC_FILE_NOT_FOUND;
            }
        }
        else{
            return RC_READ_NON_EXISTING_PAGE;
        }
    }
    else{
        return RC_FILE_NOT_FOUND;
    }
    return RC_FILE_NOT_FOUND;
}


RC shutdownBufferPool(BM_BufferPool *const bm) {
    // Check if the buffer pool exists
    if (bm == NULL || bm->mgmtData == NULL) {
        return RC_FILE_NOT_FOUND; 
    }

    // Write dirty pages to disk
    RC flushResult = forceFlushPool(bm);
    if (flushResult != RC_OK) {
        return flushResult; // Return the result of the flush operation
    }

    BM_BufferPoolManager *bufferPoolmgr = (BM_BufferPoolManager *)bm->mgmtData;
    
    //checkig for pinned pages
    int pageIndex;
    int numPages = bm->numPages;
    for(pageIndex = 0; pageIndex < numPages; pageIndex++) {
        if(bufferPoolmgr[pageIndex].fixCount != 0 ){
            return RC_FILE_NOT_FOUND; //Return error if buffer pool has pinned pages
        }
    }
    // Free memory and resources associated with the buffer pool
    free(bufferPoolmgr);

    // Reset buffer pool resources
    bm->pageFile = NULL;
    bm->strategy = -1; 
    bm->mgmtData = NULL;

    return RC_OK; // Return success

}

RC forceFlushPool(BM_BufferPool *const bm) {
    // Check if the buffer pool manager is initialized
    if (bm == NULL || bm->mgmtData == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    BM_BufferPoolManager *bmgr = (BM_BufferPoolManager *)bm->mgmtData;

    // Initialize a file handle
    SM_FileHandle fHandle;
    RC fileOpenStatus = openPageFile(bm->pageFile, &fHandle);

    if (fileOpenStatus != RC_OK) {
        // return fileOpenStatus; // Returning the openPageFile error code
        return RC_FILE_NOT_FOUND;
    }

    RC message = RC_OK; 

    int pageIndex = 0; // Initializing the counter
    int numPages = bm->numPages;
    while (pageIndex < numPages) {
        if (bmgr[pageIndex].fixCount == 0 && bmgr[pageIndex].isDirtyFlag) { 
                    RC isOpen = openPageFile(bm->pageFile, &fHandle);
                    if(isOpen != RC_OK){
                        message = RC_FILE_NOT_FOUND;
                    }
                    else{
                        int pageNum = bmgr[pageIndex].pageSerial;
                        SM_FileHandle *fH = &fHandle; 
                        SM_PageHandle memPage = bmgr[pageIndex].data;
                        RC writeResult = writeBlock(bmgr[pageIndex].pageSerial, fH, memPage);
                        if(writeResult != RC_OK){
                            message = RC_WRITE_FAILED;
                        }
                        else{
                            bmgr[pageIndex].isDirtyFlag = false;
                            writeIO++;
                        } 
                    }
        }
        pageIndex++; // Incrementing the counter
    }

    // Closing the file handle
    closePageFile(&fHandle);

    return message;
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
    RC openMessage;
    RC writeMessage;
    SM_FileHandle file;
    BM_BufferPoolManager *bmgr = (BM_BufferPoolManager *) bm->mgmtData;
     if(bm == NULL || bmgr == NULL){
        return RC_FILE_NOT_FOUND;
    }
    else if(page == NULL || page->pageNum < 0){
        return RC_READ_NON_EXISTING_PAGE;
    }
    else {
        for (int i = 0; i < bm->numPages; i++){
            if(page->pageNum == bmgr[i].pageSerial){
                openMessage = openPageFile(bm->pageFile, &file);
                if(openMessage != RC_OK)
                    return openMessage;
                writeMessage = writeBlock(bmgr[i].pageSerial, &file, bmgr[i].data);
                if(writeMessage != RC_OK){
                    return writeMessage;
                }
                bmgr[i].isDirtyFlag= FALSE;
                writeIO++;;
                return writeMessage;
            }
        }  
    }
} 

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
    BM_BufferPoolManager *bmgr = (BM_BufferPoolManager *) bm->mgmtData;
    if(bm == NULL || bmgr == NULL){
        return RC_FILE_NOT_FOUND;
    }
    else if(page == NULL || page->pageNum < 0){
            return RC_READ_NON_EXISTING_PAGE;    
    }
    else{
        for (int i = 0; i < bm->numPages; i++){
            if(page->pageNum == bmgr[i].pageSerial){
                bmgr[i].isDirtyFlag = true;
                return RC_OK;
            }
        }
    }
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
    BM_BufferPoolManager *bmgr = (BM_BufferPoolManager *) bm->mgmtData;
    if(bm == NULL || bmgr == NULL){
        return RC_FILE_NOT_FOUND;
    }
    else if(page == NULL || page->pageNum < 0){
        return RC_READ_NON_EXISTING_PAGE;
    }
    else{
        for (int i = 0; i < bm->numPages; i++){
            if(page->pageNum == bmgr[i].pageSerial ){
                bmgr[i].fixCount--;
            }
        }
        return RC_OK;       
    }
}


RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
    BM_BufferPoolManager *bmgr = (BM_BufferPoolManager *) bm->mgmtData;
    SM_FileHandle fHandle;
    RC message;
    if(bmgr == NULL || bm == NULL){
        return RC_FILE_HANDLE_NOT_INIT;
    } 
    else{
            if(bmgr[0].pageSerial == NO_PAGE){
                
                if(openPageFile(bm->pageFile, &fHandle) == RC_OK){
                    bmgr[0].data = (SM_PageHandle ) malloc(PAGE_SIZE);
                    if(ensureCapacity(pageNum+1, &fHandle) == RC_OK){
                        if(readBlock(pageNum, &fHandle, bmgr[0].data) == RC_OK){
                            frameCount = 0;
                            readIO = 0;
                            PageNumber pageSerial = pageNum;
                            bool dirtyFlag = false;
                            bmgr[0].pageSerial = pageSerial;
                            bmgr[0].isDirtyFlag = dirtyFlag;
                            bmgr[0].lastAccessedTime = frameCount;
                            bmgr[0].fixCount =  bmgr[0].fixCount + 1;
                            page->pageNum = pageNum;
                            page->data = bmgr[0].data;
                            if(closePageFile(&fHandle)== RC_OK){
                                message = RC_OK;
                            }
                        }
                        else{
                            message = RC_READ_NON_EXISTING_PAGE;
                        }
                    }
                }
            }
            else{
                int totalPages = bm->numPages;
                bool hasCapacity = true;
                for(int i=0; i<totalPages; i++){
                    
                    if(bmgr[i].pageSerial != NO_PAGE){
                        if(bmgr[i].pageSerial == pageNum){
                            page->pageNum = pageNum;
                            frameCount= frameCount+1;
                            page->data = bmgr[i].data;
                            bmgr[i].fixCount++;
                            if(bm->strategy == RS_LRU){
                                bmgr[i].lastAccessedTime = frameCount;
                            }
                            hasCapacity = false;
                            break;
                        }

                    }
                    else {
                        if(openPageFile(bm->pageFile, &fHandle) == RC_OK){
                            bmgr[i].data = (SM_PageHandle) malloc(PAGE_SIZE);
                            if(ensureCapacity(pageNum+1, &fHandle) == RC_OK){
                                if(readBlock(pageNum, &fHandle, bmgr[i].data) == RC_OK){
                                    page->pageNum = pageNum;
                                    page->data = bmgr[i].data;
                                    readIO=readIO+1;
                                    frameCount=frameCount+1;
                                    bmgr[i].pageSerial = pageNum;
                                    bmgr[i].fixCount = 1;
                                    hasCapacity = false;                
                                    
                                    if(bm->strategy == RS_LRU){
                                        bmgr[i].lastAccessedTime = frameCount;
                                    }
                                    message = closePageFile(&fHandle);
                                    if(message == RC_OK){
                                        break;
                                    }
                                    else{
                                        message = RC_FILE_NOT_FOUND;
                                    }
                                }
                            }
                        }
                    } 
                }
                if(hasCapacity == true) {
                    
                    BM_BufferPoolManager *tempBmgr = (BM_BufferPoolManager *) malloc(sizeof(BM_BufferPoolManager));
                    if(openPageFile(bm->pageFile, &fHandle)== RC_OK){
                        tempBmgr->data = (SM_PageHandle) malloc(PAGE_SIZE);
                        if(ensureCapacity(pageNum+1, &fHandle) == RC_OK){
                            message = readBlock(pageNum, &fHandle, tempBmgr->data);
                            readIO=readIO+1;
                            page->pageNum = pageNum;
                            page->data = tempBmgr->data;
                            tempBmgr->pageSerial = pageNum;
                            tempBmgr->fixCount = 1;
                            tempBmgr->isDirtyFlag = false;
                            frameCount=frameCount+1;
                            if(bm->strategy == RS_LRU){
                                tempBmgr->lastAccessedTime = frameCount;
                            }
                            
                            if(bm->strategy != RS_LRU && bm->strategy != RS_FIFO){
                                printf("Other strategies not implemented.");
                                
                            }
                            else if(bm->strategy == RS_LRU ){
                                LRUStrategy(bm, tempBmgr);
                            }
                            else{
                               FIFOStrategy(bm, tempBmgr);
                            }
                            
                        }
                        else{
                            message = RC_FILE_NOT_FOUND;
                        }
                        
                    }
                    else{
                        message = RC_FILE_NOT_FOUND;
                    }
                }
            }
           
    }
    return message;
}

PageNumber *getFrameContents(BM_BufferPool *const bm) {
    BM_BufferPoolManager *bmgr = NULL;

    int numPages = bm->numPages;

    PageNumber *pageNumber = malloc(sizeof(PageNumber) * numPages );

    if(pageNumber == NULL) {
        return NULL;
    }

    bmgr = (BM_BufferPoolManager *) bm->mgmtData;

    for (int i = 0; i < numPages; i++) {
        pageNumber[i] = (bmgr[i].pageSerial == -1) ? -1 : bmgr[i].pageSerial;

        int pageSerial = bmgr[i].pageSerial;

        if(pageSerial == -1){
            pageNumber[i] = -1;
        }

        if(pageSerial != -1){
            pageNumber[i] = bmgr[i].pageSerial;
        }

    }
    return pageNumber;
}


bool *getDirtyFlags(BM_BufferPool *const bm) {

    bool *dirtyFlag = NULL;
    BM_BufferPoolManager *bmgr = NULL;

    bmgr = (BM_BufferPoolManager *)bm->mgmtData;

    if(bmgr != NULL) {

        int numPages = bm->numPages;
        dirtyFlag = malloc(sizeof(bool) * numPages);

        if(dirtyFlag != NULL) {
            for(int i = 0; i < bm->numPages; i++) {
                int isDirtyFlag = bmgr[i].isDirtyFlag;

                if(isDirtyFlag != 0){
                    dirtyFlag[i] = true;
                }

                if(isDirtyFlag == 0){
                    dirtyFlag[i] = false;
                }

            }
        }
    }else{
        return false;
    }

    return dirtyFlag;
}

int *getFixCounts(BM_BufferPool *const bm) {
    BM_BufferPoolManager *bmgr = NULL;
    int *fixedCount = NULL;

    bmgr = (BM_BufferPoolManager *)bm->mgmtData;

    if(bmgr != NULL) {
        
        int numPages = bm->numPages;
        fixedCount = malloc(sizeof(int) * numPages);

        if(fixedCount != NULL) {

            for(int i = 0; i < numPages; i++) {
                int fixCount = bmgr[i].fixCount;

                if(fixCount == -1){
                    fixedCount[i] = 0;
                }

                if(fixCount != -1){
                    fixedCount[i] = fixCount;
                }
            }
        }
    }else{
        return false;
    }

    return fixedCount;
}

int getNumReadIO(BM_BufferPool *const bm) {
    BM_BufferPoolManager *bmgr = (BM_BufferPoolManager *) bm->mgmtData;
    if(bmgr != NULL) {
        return (int) readIO + 1;
    }
    return false; 
}

int getNumWriteIO(BM_BufferPool *const bm) {
    BM_BufferPoolManager *bmgr = (BM_BufferPoolManager *) bm->mgmtData;
    if(bmgr != NULL) {
        return (int) writeIO;
    }
    return false;
}
