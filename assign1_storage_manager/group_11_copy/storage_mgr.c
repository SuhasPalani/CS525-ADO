#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "dberror.h"

#define nullptr NULL

FILE *filePointer;
RC ret_value;



/*-----------------------------------------------
 --> Author: Arpitha Hebri Ravi Vokuda, Ramyashree Raghunandan, Rashmi Venkatesh Topannavar
 --> Function Name: setEmptyMemory
 --> Description: This function will set the memory to NULL.
 --> Parameters used is Memory
-----------------------------------------------*/
// to be done
void setEmptyMemory(char *memory){ 
    size_t i = 0;
    do {
        memory[i] = '\0';
        i++;
    } while (i < PAGE_SIZE);
}

/*-----------------------------------------------
-->Author: Arpitha Hebri Ravi Vokuda, Ramyashree Raghunandan, Rashmi Venkatesh Topannavar
--> Function: initStorageManager()
--> Description: This function initialises file pointer to null, return_code to -1. 
--> parameters used: void (no return value and no parameters)
-------------------------------------------------*/


// Function declaration to set the storage manager's initial state
void initStorageManager(void) {
    // The file pointer should be initialized to make sure it doesn't point to an already-existing file.
    filePointer = NULL; // Using NULL for pointer initialization for clarity

    // Set the default error value for the return code at initialization.
    int returnValue = -1; // For the return value, using a variable name that is more descriptive
    // Print messages to show when the initialization of the storage manager has begun and finished.
    printf("\nInitializing the Storage Manager....\n");
    // If more initialization code is required, it can be placed here.
   
    // In the case of configuring memory, launching a log file, etc.

    // Update the return value to reflect success after a successful initialization.
    returnValue = 0; // In this case, assuming 0 denotes success


    // Check if initialization was successful and print the appropriate message
    if (returnValue == 0) {
        printf("\nStorage Manager successfully initialized...\n");
    } else {
        printf("\nError initializing Storage Manager. Return code: %d", returnValue);
    }
}
// End of initStorageManager Function

/*-----------------------------------------------
-->Author: Uday Venkatesha
--> Function: createPageFile()
--> Description: This function creates a new empty file and appending that empty file into the file pointed by filePointer
--> parameters used: fileName
--> return type: Return Code
-------------------------------------------------*/

extern RC createPageFile(char *fileName)
{
    FILE *filePtr = fopen(fileName, "w+");

    // Check if the file was successfully opened for read and write
    if (filePtr == NULL)
    
        return RC_FILE_NOT_FOUND;

    // Allocate memory for an empty page
    SM_PageHandle emptyPage = (SM_PageHandle)malloc(PAGE_SIZE);
    
    // Ensure memory allocation was successful
    if (emptyPage == NULL)
    {
        fclose(filePtr);
        return RC_WRITE_FAILED;  // Use a generic write failure code
    }

    // Initialize the empty page with null bytes
    memset(emptyPage, 0, PAGE_SIZE);

    // Write the empty page to the file
    size_t writeResult = fwrite(emptyPage, sizeof(char), PAGE_SIZE, filePtr);

    if (writeResult == PAGE_SIZE)
    {
        printf("Empty page successfully written to file.\n");
    }
    else
    {
        printf("Failed to write empty page to file.\n");
    }

    // Clean up: free allocated memory and close the file
    free(emptyPage);
    fclose(filePtr);

    return RC_OK;
}


  


/*-----------------------------------------------
--> Author: Nishchal Gante Ravish
--> Function: openPageFile()
--> Description: This function opens the file in read mode using file handle
--> parameters used: The params we used here are fileName of char type and file handle
-------------------------------------------------*/


extern RC openPageFile(char* fileName, SM_FileHandle *fHandle) {
    // This is used to open the given file in read and update mode
    FILE *fpf = fopen(fileName, "r+");

    // This is written so if the file isn't opened it'll return a file not found
    if (!fpf) {
        return RC_FILE_NOT_FOUND; // File couldn't be opened
    }

    // Now let us initialize the file handle details

    //This is used to start at the initial page
    fHandle -> curPagePos=0;

    // The coed to trace fp
    fHandle->fileName = fileName;

    // Now go to eof to check len
    fseek(fpf, 0, SEEK_END);

    // Total the no of pages
    int fzb = ftell(fpf);
    fHandle->totalNumPages = (fzb + PAGE_SIZE - 1) / PAGE_SIZE;


    // Now we'll close the files
    fclose(fpf);

    // Now let us return the val

    return RC_OK;
}
/*-------------------------------------------------
 --> Author: Rashmi Venkatesh Topannavar
 --> Function Name: closePageFile
 --> Description: This function closes the opened page file.
 --> Parameters:  File Handle
-------------------------------------------------*/


extern RC closePageFile(SM_FileHandle *fHandle) {
    // Check if the file handle and its fileName are valid
    if (fHandle == NULL || fHandle->fileName == NULL) {
        printf("Invalid file handle or file name.\n");
        return RC_FILE_NOT_FOUND; // Or another appropriate error code for invalid handle
    }

    // Open the file in read mode to get the FILE* (this is not an ideal approach but may be necessary if the design requires it)
    FILE *file = fopen(fHandle->fileName, "r");
    if (file == NULL) {
        printf("Cannot open file: %s\n", fHandle->fileName);
        return RC_FILE_NOT_FOUND; // Or another appropriate error code for file not found or cannot open
    }

    // Close the file
    if (fclose(file) == 0) {
        printf("File closed successfully: %s\n", fHandle->fileName);
        return RC_OK; // Success
    } else {
        printf("Failed to close the file: %s\n", fHandle->fileName);
        return RC_FILE_NOT_FOUND; // Or another appropriate error code for close failure
    }
}


/*-------------------------------------------------
 --> Author: Rashmi Venkatesh Topannavar
 --> Function Name: destroyPageFile
 --> Description: This function deletes the page file
 --> Parameters : File name
 --> Return type: Return Code
-------------------------------------------------*/

extern RC destroyPageFile(char *fileName)
{
	
    filePointer = fopen(fileName, "r");

    ret_value = (filePointer != 0 && remove(fileName) == 0) ? RC_OK : RC_FILE_NOT_FOUND;
    return ret_value;

}









/*-----------------------------------------------
-->Author: Nishchal Gante Ravish
--> Function: readBlock()
--> Description: This func is used to read a specific page from teh given file
--> parameters used: int pgeNum, SM_FileHandle *fHandle, SM_PageHandle mrPg
--> return type: Return the success or failure operation using status code
-------------------------------------------------*/

// The function starts in the below code

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {

    // Init file handle so we can interact with the given file

    if(fHandle == NULL) {
        // If handle isn't initialized, return error
        return RC_FILE_HANDLE_NOT_INIT;
    }




    if(pageNum < 0 || fHandle->totalNumPages <= pageNum) {
        // If no page encountered, return no page error
        return RC_READ_NON_EXISTING_PAGE;
    }

    // Use to track 
    fHandle->curPagePos = pageNum;

    // Open the file
    FILE *filePointer = fopen(fHandle->fileName, "r");
    

    // Return error if failed opening


    if(filePointer == NULL) {
        return RC_FILE_NOT_FOUND;
    }




    // Calucalte offset 
    long val_off = (long)pageNum * PAGE_SIZE;


    // Used to go to the start of the page 

    int pos_set = fseek(filePointer, val_off, SEEK_SET);

    
    if (pos_set != 0) {


        // Close the file iff the eroro comes
        fclose(filePointer);
        return RC_READ_NON_EXISTING_PAGE;
    }



    // Read content of page
    fread(memPage, sizeof(char), PAGE_SIZE, filePointer);



    // Close given file so we can utilize memory resources
    fclose(filePointer);

   
   
   // Return the gievn status code
    return RC_OK;
}

/*-----------------------------------------------
-->Author: Arpitha Hebri Ravi Vokuda
--> Function: getBlockPos()
--> Description: This function is used to return the current block position in file
--> parameters used: SM_FileHandle *fHandle
--> return type: Return Code
-------------------------------------------------*/

RC getBlockPos (SM_FileHandle *fHandle)
{
    /*Current Position of the pointer has to be returned */

    int curPosPtr = 0;
	
    switch (1) {
        case 1:
            ret_value = (fHandle == NULL) ? RC_FILE_NOT_FOUND : (curPosPtr = fHandle->curPagePos);
            break;
    }

    return ret_value;
}



/*-----------------------------------------------
--> Author: Ramyashree Raghunandan
--> Function: readFirstBlock()
--> Description: This function opens the file in reads the first block of the file
--> parameters used:SM_FileHandle *fileHandle, SM_PageHandle mrPg
-------------------------------------------------*/

RC readFirstBlock(SM_FileHandle *fileHandle, SM_PageHandle mrPg) {
    //Opening the file in read mode
    FILE *filePointer = fopen(fileHandle->fileName, "r");
    RC status = (filePointer == nullptr) ? RC_FILE_NOT_FOUND : RC_OK;
    //Checking if file opened successfully
    if (status == RC_OK) {
        (fseek(filePointer, 0, SEEK_SET)) ? (fprintf(stderr, "fseek() failed in file storage_mgr.c\n"), fclose(filePointer), EXIT_FAILURE) : 0;
        fread(mrPg, sizeof(char), PAGE_SIZE, filePointer);
        if (feof(filePointer)) {
            printf("Error reading: unexpected end of file\n");
            fclose(filePointer);
            return EXIT_FAILURE;
        }
        fileHandle->curPagePos = 0;
    }
    if (filePointer != nullptr) {
        fclose(filePointer);
    }
    return status;
}




/*-----------------------------------------------
--> Author: Nishchal Gante Ravish
--> Function: readPreviousBlock()
--> Description: The below func is used to read the mem block using memPage
--> parameters used: File handle and page handle
--> return type: Return the status code
-------------------------------------------------*/



RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {


    // Now letus check if handle is work fine

    if (fHandle != NULL && fHandle->curPagePos > 0) {

        // if valid then 
        // read each block and dec the pos
    return readBlock(fHandle->curPagePos - 1, fHandle, memPage);
    }


    else{
        // Put file not found if pos is in the begining or null

        return RC_FILE_NOT_FOUND;
    }


}


/*----------------------------------------------------------------------------
--> Author: Rashmi Venkatesh Topannavar
--> Function Name: readCurrentBlock
--> Description: This function reads the current block into the memory address mrPg
--> Parameters: SM_FileHandle *fHandle, SM_PageHandle mrPg
--> Return type: Return Code
-----------------------------------------------------------------------------*/


RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /*Using the present blocks poosition, we read it's contents*/
    
    return (fHandle != nullptr) ? readBlock(fHandle->curPagePos, fHandle, mrPg) : RC_FILE_NOT_FOUND;
}

/*-----------------------------------------------
-->Author: Arpitha Hebri Ravi Vokuda
--> Function: readNextBlock()
--> Description:  This is used to read the next block in to memory address mrPg
--> parameters used: SM_FileHandle *fHandle, SM_PageHandle mrPg
--> return type: Return Code
-------------------------------------------------*/

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    
switch (1) {
    case 1:
        switch (fHandle != nullptr) {
            case 1:
                return readBlock(fHandle->curPagePos + 1, fHandle, mrPg);
            case 0:
                return RC_FILE_NOT_FOUND;
        }
        break;
}

}


/*-----------------------------------------------
--> Author: Ramyashree Raghunandan
--> Function: readLastBlock()
--> Description: This function reads the last block of the page using memory address mrPg
--> parameters used:SM_FileHandle *fileHandle, SM_PageHandle mrPg
--> return type: Return Code
-------------------------------------------------*/

RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle mrPg) {

    return (fHandle != nullptr)
        ? readBlock((fHandle->totalNumPages > 0) ? (fHandle->totalNumPages - 1) : 0, fHandle, mrPg)
        : RC_FILE_NOT_FOUND;
}

/*-----------------------------------------------
 --> Author: Rashmi Venkatesh Topannavar
 --> Function Name: writeBlock
 --> Description: This function will write the data into the specified page number of the file.
 --> Parameters:  pageNum, SM_FileHandle *fHandle, SM_PageHandle mrPg
 --> Return type: Return Code
-------------------------------------------------*/

RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle mrPg) {
    int offset_write = 0;
    int page_total = 0;

    // Check if the file handle and page number are valid
    if (fHandle == 0 || pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_WRITE_FAILED;
    }

    // Calculate the write offset based on the page number
    offset_write = pageNum * PAGE_SIZE;

    // Open the file in "w+" mode to allow reading and writing
    filePointer = fopen(fHandle->fileName, "w+");

    if (filePointer == 0) {
        return RC_FILE_NOT_FOUND;
    }

    // Move the file pointer to the specified position
    if (fseek(filePointer, offset_write, SEEK_SET) != 0) {
        fclose(filePointer);
        return RC_WRITE_FAILED;
    }

    // Write data from mrPg to the file
    if (fwrite(mrPg, sizeof(char), PAGE_SIZE, filePointer) != PAGE_SIZE) {
        fclose(filePointer);
        return RC_WRITE_FAILED;
    }

    // Update the curPagePos and totalNumPages in the file handle
    fHandle->curPagePos = pageNum;
    
    // Calculate the total number of pages in the file
    fseek(filePointer, 0, SEEK_END);
    page_total = ftell(filePointer) / PAGE_SIZE;
    fHandle->totalNumPages = page_total;

    // Close the file
    fclose(filePointer);

    return RC_OK;
}

/*-----------------------------------------------
 --> Author: Rashmi Venkatesh Topannavar
 --> Function Name: writeCurrentBlock
 --> Description: This function will write the data into the current block which the file handle is accessing
 --> Parameters : SM_FileHandle *fHandle, SM_PageHandle mrPg
 --> Return type: Return Code
-----------------------------------------------*/


RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /* Writing the mrPg data into the file by passing the curPagePos to the writeblock */

    int current_Block = (fHandle == 0) ? RC_FILE_NOT_FOUND : fHandle->curPagePos;
    return writeBlock(current_Block, fHandle, mrPg);
        	
}

/*-------------------------------------------------
 --> Author: Ramyashree Raghunandan
 --> Function Name: appendEmptyBlock
 --> Description: This function will append an empty page to the file.
 --> Parameters used are SM_FileHandle *fHandle
---------------------------------------------------*/

RC appendEmptyBlock(SM_FileHandle *fHandle) 
{
    // To append an empty block, the file poniter has to seek to eof and 
    // new page has to added to the file. Later the total number of pages is increased by 1.

    int page_total = 0;
    if(fHandle != nullptr)                                                     
    {
        // Creating an empty page

        char *empty_Block = malloc(PAGE_SIZE * sizeof(char));
        setEmptyMemory(empty_Block); 

        filePointer = fopen(fHandle->fileName, "w+");

        return (filePointer != nullptr) ? ((fseek(filePointer, 0, SEEK_END) == 0 && fwrite(empty_Block, 1, PAGE_SIZE, filePointer) != 0) ?
                (fHandle->totalNumPages++, fHandle->curPagePos = fHandle->totalNumPages - 1, free(empty_Block), fclose(filePointer), RC_OK) :
                (fclose(filePointer), RC_WRITE_FAILED)) : RC_FILE_NOT_FOUND;

        fclose(filePointer);
                               
    }
    
    return RC_FILE_NOT_FOUND; 
    	
}

/*----------------------------------------------------------
 --> Author: Arpitha Hebri Ravi Vokuda
 --> Function Name: ensureCapacity()
 --> Description: This function make sure that the number of pages required, are available in the file.
 --> Parameters used are SM_FileHandle *fHandle and number of Pages
 --> Return type: Return Code
------------------------------------------------------------*/

RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) 
{
    int No_of_pg = 0;
    int i;
    
switch (1) {
        case 1:
            if (fHandle != nullptr) {
                No_of_pg = numberOfPages - (*fHandle).totalNumPages;

                switch (No_of_pg > 0) {
                    case 1:
                    	//if the number of pages is greater than total number of pages 
						i = 0;
   						loop_start:
    					if (i < No_of_pg) {
        				appendEmptyBlock(fHandle);
        				i++;
        				goto loop_start;
    					}
    					ret_value = RC_OK;
                        break;

                    case 0:
                        // If the number of pages is within the total number of pages.
                        ret_value = RC_WRITE_FAILED;  // Changed error code
                        break;
                }
            } else {
                ret_value = RC_FILE_HANDLE_NOT_INIT;  // Changed error code
            }
            break;
    }

    return ret_value;
}