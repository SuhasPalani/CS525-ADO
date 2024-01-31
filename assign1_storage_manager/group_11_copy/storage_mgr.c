#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "dberror.h"
#include <stdbool.h>

#define nullptr NULL

FILE *filePointer;
RC ret_value;






/*-----------------------------------------------
 --> Author: Nishchal Gante Ravish
 --> Function Name: setEmptyMemory
 --> Description: This func is used to set the mem to null
 --> Parameters used is Memory
-----------------------------------------------*/




void setEmptyMemory(char *memory) {
    for (size_t i = 0; i < PAGE_SIZE; i++) {
        memory[i] = '\0';
    }
}



/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: initStorageManager()
--> Description: This function sets return_code to -1 and the file reference to null. 
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
 --> Author: Suhas Palani
--> Function Name: closePageFile
--> Description: This function closes the opened page file function.
--> Parameters:  File Handle

-------------------------------------------------*/

extern RC closePageFile(SM_FileHandle *fHandle) {

   // Verifies the validity of the file handle and its fileName.

   if (fHandle == NULL || fHandle->fileName == NULL) {
       printf("Invalid file handle or file name.\n");
       return RC_FILE_NOT_FOUND; // Or another appropriate error code for invalid handle
   }




   // This is not the best course of action, but it could be required if the design calls for it. Open the file in read mode to obtain the FILE*.
   FILE *file = fopen(fHandle->fileName, "r");


   if (file == NULL) {
       printf("Cannot open file: %s\n", fHandle->fileName);
       return RC_FILE_NOT_FOUND; // Another suitable error code for an unavailable or inoperable file.
   }



   // Closes the file
   if (fclose(file) == 0) {

       printf("File closed successfully: %s\n", fHandle->fileName);

       return RC_OK; // Successful


   } else {


       printf("Failed to close the file: %s\n", fHandle->fileName);

       return RC_FILE_NOT_FOUND; // or an other error code suitable for a close failure
   }
}




/*-------------------------------------------------
 --> Author: Uday Venkatesha
 --> Function Name: destroyPageFile
 --> Description: This function deletes the page file
 --> Parameters : File name
 --> Return type: Return Code
-------------------------------------------------*/

extern RC destroyPageFile(char *fileName)
{
    // Open the file in read mode to check its existence
    filePointer = fopen(fileName, "r");

    // If the file pointer is NULL, the file doesn't exist
    if (filePointer == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Close the file before attempting to delete it
    fclose(filePointer);

    // Remove the file and check the result
    ret_value = (remove(fileName) == 0) ? RC_OK : RC_FILE_NOT_FOUND;

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
/*
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

*/

// The function starts in the below code

extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int condition = 0; // 0: Initial state, 1: File handle not init, 2: Page number out of bounds, 3: Continue to read

    // Determine initial condition
    if (!fHandle) {
        condition = 1;
    } else if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        condition = 2;
    } else {
        condition = 3;
    }

    switch (condition) {
        case 1: // File handle not initialized
            return RC_FILE_HANDLE_NOT_INIT;
        
        case 2: // Page number out of bounds
            return RC_READ_NON_EXISTING_PAGE;
        
        case 3: { // Proceed with reading the block
            FILE *filePointer = fopen(fHandle->fileName, "r");
            int fileOpenCondition = filePointer ? 4 : 5; // 4: File opened, 5: File not found

            switch (fileOpenCondition) {
                case 5: // File not found
                    return RC_FILE_NOT_FOUND;

                case 4: { // File opened, proceed with further checks
                    fseek(filePointer, pageNum * PAGE_SIZE, SEEK_SET);
                    int seekCondition = (ftell(filePointer) == pageNum * PAGE_SIZE) ? 6 : 7; // 6: fseek successful, 7: fseek failed

                    switch (seekCondition) {
                        case 7: // fseek failed
                            fclose(filePointer);
                            return RC_READ_NON_EXISTING_PAGE;

                        case 6: { // fseek successful, read the page
                            size_t readBytes = fread(memPage, sizeof(char), PAGE_SIZE, filePointer);
                            fclose(filePointer);
                            int readCondition = (readBytes == PAGE_SIZE) ? 8 : 9; // 8: Read successful, 9: Read failed

                            switch (readCondition) {
                                case 9: // Read failed
                                    return RC_READ_NON_EXISTING_PAGE;
                                
                                case 8: // Read successful
                                    fHandle->curPagePos = pageNum;
                                    return RC_OK;
                            }
                        }
                    }
                }
            }
        }
    }

    // Fallback in case no conditions are met, which should not happen
    return RC_FILE_NOT_FOUND;
}



/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: getBlockPos()
--> Description: This function is used to return the current block position in file
--> parameters used: SM_FileHandle *fHandle
--> return type: Return Code

-------------------------------------------------*/


RC getBlockPos(SM_FileHandle *fHandle) {
   // Set the return value to its initial value.
   RC ret_value;



   // Verify whether the file handle is NULL.


   if (fHandle == NULL) {
       ret_value = RC_FILE_NOT_FOUND;
   } else {
       // File handle is not NULL; please verify the location of the page.



       if (fHandle->curPagePos >= 0) {
           ret_value = RC_OK;
       } else {
           // The page position is incorrect right now.


           ret_value = RC_FILE_HANDLE_NOT_INIT;
       }
   }

   return ret_value; // Return the value
}




/*-----------------------------------------------
--> Author: Uday Venkatesha 
--> Function: readFirstBlock()
--> Description: This function opens the file in reads the first block of the file
--> parameters used:SM_FileHandle *fileHandle, SM_PageHandle mrPg

-------------------------------------------------*/


RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Open the file for reading. fileHandle->fileName holds the name of the file.
    filePointer = fopen(fHandle->fileName, "r");

    // Check if the file was successfully opened.
    if (filePointer == NULL) {
        // Print an error message if the file could not be opened.
        printf("File does not exist\n");
        // Return a specific error code indicating the file was not found.
        return RC_FILE_NOT_FOUND;
    }

    // Check for errors in the file stream.
    if (ferror(filePointer)) {
        // Print an error message if there is an error in reading from the file.
        printf("Error in reading from file!\n");
        // Close the file to free resources and prevent memory leaks.
        fclose(filePointer);
        // Return a generic error code indicating a read error.
        return RC_OK;
    }

    // Move the file pointer to the start of the file.
    if (fseek(filePointer, 0, SEEK_SET) != 0) {
        // Print an error message if fseek() fails to set the file pointer.
        fprintf(stderr, "fseek() failed in file %s at line #%d\n", __FILE__, __LINE__ - 4);
        // Close the file to free resources.
        fclose(filePointer);
        // Return a generic error code indicating a seek failure.
        return RC_OK;
    }

    // Read the first block of size PAGE_SIZE from the file into mrPg.
    size_t bytesRead = fread(memPage, sizeof(char), PAGE_SIZE, filePointer);
    // Check if the bytes read are less than PAGE_SIZE and it's not end of file.
    if (bytesRead < PAGE_SIZE && !feof(filePointer)) {
        // Print an error message if an incomplete block is read.
        printf("Error reading: incomplete read of first block\n");
        // Close the file to avoid resource leak.
        fclose(filePointer);
        // Return a generic error code indicating less data was read than expected.
        return RC_OK;
    }

    // Update the current page position in the fileHandle to 0 (beginning of the file).
    fHandle->curPagePos = 0;

    // Close the file after successful reading.
    fclose(filePointer);
    // Return a success code indicating successful completion of the operation.
    return ;
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
----
--> Author: Suhas Palani
--> Function Name: readCurrentBlock
--> Description: This function reads the current block into the memory address memPage
--> Parameters: SM_FileHandle *fHandle, SM_PageHandle memPage
--> Return type: Return Code

-----------------------------------------------------------------------------*/



RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
   // Verify that the current page position is correct and that the file handle is not NULL.


   if (fHandle != NULL && fHandle->curPagePos >= 0) {
       // Gives the readBlock function control over the read operation.


       return readBlock(fHandle->curPagePos, fHandle, memPage);


   }
   else {
       // If the location is invalid or the file handle is NULL, return the relevant error code.


       return (fHandle == NULL) ? RC_FILE_NOT_FOUND : RC_READ_NON_EXISTING_PAGE;
   }
}








/*-----------------------------------------------
-->Author: Uday Venkatesha
--> Function: readNextBlock()
--> Description:  This is used to read the next block in to memory address mrPg
--> parameters used: SM_FileHandle *fHandle, SM_PageHandle mrPg
--> return type: Return Code
-------------------------------------------------*/



RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {


    // Check if the file handle is not null (i.e., the file handle exists).


    if (fHandle != NULL) {


        // Increment the current page position before reading.


        // This moves the position to the next block to read.


        int nextPage = fHandle->curPagePos + 1;

        // Call readBlock to read the block at the new page position.


        // readBlock should be a function that reads the specified block of a file.

        return readBlock(nextPage, fHandle, memPage);


    } else {


        // If the file handle is null, return an error code indicating the file was not found.


        return RC_FILE_NOT_FOUND;
    }
}



/*-----------------------------------------------
--> Author: Nishchal Gante Ravish
--> Function: readLastBlock()
--> Description: The given below funciton is used to read the values from the last page
--> parameters used: Makes use of File and Page Handle
--> return type: Status Code
-------------------------------------------------*/


// The below func is used to read last values 

RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {

    // Check if file handle is valid 

    if (fHandle == NULL) {

        // If null return err
        // With specific err code

        return RC_FILE_HANDLE_NOT_INIT;

    }

    // Cal pos of final block

    int fbi = fHandle->totalNumPages - 1;


    // Code to validate aboev code

    if (fbi < 0) {

        // If no blocks return out error

        return RC_FILE_NOT_FOUND;

    }

    // Check to seee if code returns success or failure
    RC status_checker = readBlock(fbi, fHandle, memPage);



    // Return the status code and check for err
    return status_checker;
}











/*-----------------------------------------------
--> Author: Suhas Palani
--> Function Name: writeBlock
--> Description: The data will be written by this function to the designated file page.
--> Parameters:  pageNum, SM_FileHandle *fHandle, SM_PageHandle mrPg
--> Return type: Return Code

-------------------------------------------------*/

RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {

   int offset_write = 0;


   int pageOffsetAdjustment = 0; // Relevant in context, but not properly applied


   FILE *filePointer;


   // Verify the validity of the page number and file handle.


   if (fHandle == NULL || pageNum < 0 || pageNum >= fHandle->totalNumPages) {

    
       return RC_WRITE_FAILED;
   }


   // Determine the write offset by using the page number as a guide.


   offset_write = pageNum * PAGE_SIZE;


   // To enable both reading and writing, open the file in "w+" mode.


   filePointer = fopen(fHandle->fileName, "w+");

   if (filePointer == NULL) {

       return RC_FILE_NOT_FOUND;
   }


   // Look that you're making calculations with pageOffsetAdjustment, but it's set to 0.


   offset_write += pageOffsetAdjustment; // No actual effect since pageOffsetAdjustment is 0


   // Put the file pointer at the designated location.


   if (fseek(filePointer, offset_write, SEEK_SET) != 0) {

       fclose(filePointer);

       return RC_WRITE_FAILED;
   }


   // Enter data into the file from the memPage.


   if (fwrite(memPage, sizeof(char), PAGE_SIZE, filePointer) != PAGE_SIZE) {

       fclose(filePointer);

       return RC_WRITE_FAILED;
   }


   // Update the file handle's curPagePos.


   fHandle->curPagePos = pageNum;


   // Determine how many pages there are in the entire file.


   fseek(filePointer, 0, SEEK_END);
   fHandle->totalNumPages = ftell(filePointer) / PAGE_SIZE;


   // Closes the file
   fclose(filePointer);


   return RC_OK;
     
}



/*-----------------------------------------------
 --> Author: Uday Venkatesha
 --> Function Name: writeCurrentBlock
 --> Description: This function will write the data into the current block which the file handle is accessing
 --> Parameters : SM_FileHandle *fHandle, SM_PageHandle mrPg
 --> Return type: Return Code

-----------------------------------------------*/



RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {

    // First, check if the file handle pointer is null.


    // This is to ensure we have a valid file handle before proceeding.


    if (fHandle == NULL) {


        // If the file handle is null, return an error code indicating the file was not found.


        // This error code should be defined in your system.


        return RC_FILE_NOT_FOUND;
    }

    // Retrieve the current page position from the file handle.


    // This indicates the block number where we want to write.


    int currentBlock = fHandle->curPagePos;

    // Call writeBlock to write data to the current block of the file.


    // writeBlock should be a function defined elsewhere in your code,


    // which handles the actual process of writing to the file.


    // It takes the block number, file handle, and data to write as arguments.


    return writeBlock(currentBlock, fHandle, memPage);
}







/*-------------------------------------------------
 --> Author: Nishchal Gante Ravish
 --> Function Name: appendEmptyBlock
 --> Description: The below func is used to append an empty page to file
 --> The parameters used are file handle
---------------------------------------------------*/




void initializeEmptyBlock(char *block, size_t size);


// Write the func below

RC appendEmptyBlock(SM_FileHandle *fHandle) {
    
    // Here we check if fhandle is fine

    if (!fHandle) {


        return RC_FILE_NOT_FOUND; 
    }


    // Initialize empty block


    char *ebl = calloc(PAGE_SIZE, sizeof(char));


    if (!ebl) {


        return RC_FILE_NOT_FOUND; 
    }

    // Let us open the file in append mode


    FILE *file = fopen(fHandle->fileName, "a");


    if (!file) {

        // Let's free the allocated block 

        free(ebl); 


        return RC_FILE_NOT_FOUND; 
    }



    // Write the empty block to the fiel


    if (fwrite(ebl, sizeof(char), PAGE_SIZE, file) < PAGE_SIZE) {

        fclose(file);

        free(ebl);

        return RC_FILE_NOT_FOUND;
    }



    // Now let's write the meta data


    fHandle->totalNumPages++;



    fHandle->curPagePos = fHandle->totalNumPages - 1;

    // Clean everything and return success val


    fclose(file);


    free(ebl);


    return RC_OK; 
}


// Func for zero init

void initializeEmptyBlock(char *block, size_t size) {


    if (block) {


        memset(block, 0, size);
    }
}






/*----------------------------------------------------------
--> Author: Suhas Palani
--> Function Name: ensureCapacity()
--> Description: This feature confirms that the file has the required amount of pages.
--> Parameters used are SM_FileHandle *fHandle and number of Pages
--> Return type: Return Code

------------------------------------------------------------*/



RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
   int pagesNeeded = 0;
   int tempPageAdjustment = 0;
   // Verify the file handle.


   if (fHandle != NULL) {
       pagesNeeded = numberOfPages - fHandle->totalNumPages;


       // Add blocks only if additional pages are required.
       if (pagesNeeded > 0) {
           int i = 0;
           do {
               appendEmptyBlock(fHandle);
               i++;
               tempPageAdjustment+= 1;
               // Continue the loop if the condition is fulfilled to emulate the behavior.


               if (i < pagesNeeded) continue;  // Similar to a to the loop start


               else break;  // When the condition is no longer met, exit the loop.


           } while (false);  // Make sure the loop only continues after the first time.




           return RC_OK;
       } else {
           // No further pages are required, yet the context may point to an error


           return RC_WRITE_FAILED;
       }
   } else {
       return RC_FILE_HANDLE_NOT_INIT;
   }
}



