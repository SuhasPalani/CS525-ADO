#include "dberror.h"
#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Programmed by Akash Didigi Kashinath; CWID: A20524076

extern void initStorageManager (){
}

FILE *filePointer;

// Function to create a new page file with the given fileName
extern RC createPageFile(char *fileName) {
    RC createPageFileMsg; // Variable to store error code 
     filePointer = fopen(fileName, "w");
     
    char *pageBuffer;

    if (filePointer) {
        // Allocating some memory for the blank page
        pageBuffer = malloc(PAGE_SIZE * sizeof(char));
        
        // Writing to the blank page to the file
        fwrite(pageBuffer, sizeof(char), PAGE_SIZE, filePointer);
        
        // Free the memory allocated for the blank page
        free(pageBuffer);
        
        // Close the file
        fclose(filePointer);
        
        createPageFileMsg = RC_OK;
    } 
    else {
        createPageFileMsg = RC_FILE_NOT_FOUND;

    }
    return createPageFileMsg;
}

// Function to open an existing page file with the given fileName
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    RC openPageFileMsg;
    filePointer = fopen(fileName, "r+");

    if (filePointer) {
        
        int bufferSize = ftell(filePointer);
        char *tempFileName = fileName; 
        int tempTotalNumPages = bufferSize+1; 
        int tempCurPagePos = fseek(filePointer, 0, SEEK_SET); 
        void *tempMgmtInfo = NULL; 

        // Initialize the file handle
        fHandle->fileName = fileName;
        fHandle->totalNumPages = tempTotalNumPages;
        fHandle->curPagePos = tempCurPagePos;
        fHandle->mgmtInfo = tempMgmtInfo;

        openPageFileMsg = RC_OK;
        fclose(filePointer);
    } else {
        openPageFileMsg = RC_FILE_NOT_FOUND;
    }
    return openPageFileMsg;
}

// Function to close a page file and clean up the file handle
extern RC closePageFile(SM_FileHandle *fHandle) {

    RC closePageFileMsg;
    
    if (fHandle) {
        filePointer = fopen(fHandle->fileName, "r");
        if (filePointer) {
            // Close the file
            int closeResult = fclose(filePointer);

            if(closeResult == 0){
                char *temFileName = NULL;
                int tempTotalNumPages = 0;
                int tempCurPagePos = 0;
                void *tempMgmtInfo = NULL;
                
                // Assign the temporary variables to fHandle fields
                fHandle->fileName = temFileName; 
                fHandle->totalNumPages = tempTotalNumPages; 
                fHandle->curPagePos = tempCurPagePos; 
                fHandle->mgmtInfo = tempMgmtInfo;

                closePageFileMsg = RC_OK; 
            }
            else {
                closePageFileMsg = -1;
            }
        }
        else {
            closePageFileMsg = RC_FILE_NOT_FOUND;
        } 
    } 
    else {
        closePageFileMsg = RC_FILE_HANDLE_NOT_INIT;
    }
    return closePageFileMsg;
}

// Function to destroy (delete) a page file with the given fileName
extern RC destroyPageFile(char *fileName) {
    RC destroyPageFileMsg;
    printf("Destroying %s\n", fileName);

    if (remove(fileName) != 0) { // Checking if file was deleted; If successful it returns 0 else -1
     destroyPageFileMsg = RC_FILE_NOT_FOUND; // Assign 1 if file was not found or if there was any error while deleting
    } 
    else {
       destroyPageFileMsg = RC_OK; // Assign 0 if delete operation was successful
    }
    return destroyPageFileMsg; // Return final output
}


// Programmed by Harshitha Satish Reddy; CWID: A20547093

// Function to read a block from the file 
extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Checking the if file handle is initialized
    if(fHandle == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    // Moving curPagePos to the page that is being read
    fHandle->curPagePos = pageNum;

    filePointer = fopen(fHandle->fileName,"r");
    if(filePointer == NULL) {
        return RC_FILE_NOT_FOUND;
    }
    long offset = pageNum * PAGE_SIZE;
    int whence = SEEK_SET;
    int setPos = fseek(filePointer, offset, whence); 
    if (setPos != 0) {
        fclose(filePointer);
        return RC_READ_NON_EXISTING_PAGE;
    }
    fread(memPage, sizeof(char), PAGE_SIZE, filePointer);
    fclose(filePointer);
    return RC_OK;
}

// Function to get the current page position of a file
extern int getBlockPos (SM_FileHandle *fHandle) {
    if(fHandle == NULL){
       return -1; //Return -1 if fHandle is empty
    }
    return fHandle->curPagePos;
}

// Function to read first block from the file
extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int firstBlock = 0; //First block appears at the 0th page
    return readBlock(firstBlock, fHandle, memPage);
}

// Function to read the previous block from current position in the file
extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    int previousBlock = fHandle->curPagePos - 1;
    return readBlock(previousBlock, fHandle, memPage);
}

// Function to read the current block from the file
extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int currentBlock = fHandle->curPagePos;
    return readBlock(currentBlock, fHandle, memPage);
}

// Function to read the next block from current position in the file
extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int nextBlock = fHandle->curPagePos+1;
    return readBlock(nextBlock, fHandle, memPage);
}

// Function to read last block from the file
extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int lastBlock = fHandle->totalNumPages - 1; 
    return readBlock(lastBlock, fHandle, memPage);
}

// Programmed by Nikhil Goud; CWID: A20527738

// Function to write content to a specific block in the page file
extern RC writeBlock(int currentPageNum, SM_FileHandle *sMFHandle, SM_PageHandle currentMemPage) {
    FILE *filePointer;
    RC statusMsg;
    
    if (sMFHandle == NULL) {
        statusMsg = RC_FILE_HANDLE_NOT_INIT;
        return statusMsg;
    }

    filePointer = fopen(sMFHandle->fileName, "r+");

    if (filePointer == NULL) {
        statusMsg = RC_FILE_NOT_FOUND;
        return statusMsg;
    }
    long offset = currentPageNum * PAGE_SIZE;
    int whence = SEEK_SET;
    int response = fseek(filePointer,offset , whence);
    if (response != 0) {
        fclose(filePointer);
        return RC_WRITE_FAILED;
    }

    fwrite(currentMemPage, sizeof(char), strlen(currentMemPage), filePointer);
    fclose(filePointer);
    return RC_OK;
}


// Function to write content to the current block in the page file
extern RC writeCurrentBlock(SM_FileHandle *sMFHandle, SM_PageHandle memPage){
    return writeBlock(sMFHandle->curPagePos, sMFHandle, memPage);
}

// Function to append an empty block to the page file
extern RC appendEmptyBlock(SM_FileHandle *sMFHandle) {
    if (sMFHandle == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    FILE *filePointer = fopen(sMFHandle->fileName, "r+");
    if (filePointer == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    int totalPages = sMFHandle->totalNumPages;

    SM_PageHandle *newEmptyBlock = (SM_PageHandle*) malloc(PAGE_SIZE * sizeof(char));
    if (newEmptyBlock == NULL) {
        fclose(filePointer);
        return RC_WRITE_FAILED;
    }

    long offset = 0;
    int whence = SEEK_END;

    int seekResult = fseek(filePointer, offset, whence);
    if (seekResult != 0) {
        fclose(filePointer);
        free(newEmptyBlock);
        return RC_WRITE_FAILED;
    }

    if (fwrite(newEmptyBlock, sizeof(char), PAGE_SIZE, filePointer) != PAGE_SIZE) {
        return RC_WRITE_FAILED;
    }

    fclose(filePointer);
    free(newEmptyBlock);

    sMFHandle->totalNumPages = totalPages + 1;

    return RC_OK;
}

// Function to simulate ensuring a minimum number of pages in the file
extern RC ensureCapacity (int numberOfPages, SM_FileHandle *sMFHandle){
    RC statusMsg;
    int totalPages = sMFHandle->totalNumPages;

    if(totalPages > numberOfPages){
        return RC_OK;
    }
    else{
        SM_PageHandle *ep = (SM_PageHandle*) malloc((numberOfPages - totalPages) * PAGE_SIZE * sizeof(char));
        free(ep);
        return RC_OK;
    }
}