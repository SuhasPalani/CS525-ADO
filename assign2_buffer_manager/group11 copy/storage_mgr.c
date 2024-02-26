#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "dberror.h"
#include <stdbool.h>


#define nullptr NULL

FILE *filePointer;
RC ret_value;

int block_num = 0;
int total_offset = 1;


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
-->Author: Arpitha Hebri Ravi Vokuda
--> Function: createPageFile()
--> Description: This function creates a new empty file and appending that empty file into the file pointed by filePointer
--> parameters used: fileName
--> return type: Return Code
-------------------------------------------------*/

extern RC createPageFile ( char * fileName )
{
    block_num = 0;
    filePointer = fopen(fileName,"w+"); 
	
    //opening the file with write w+ mode, hence creating a empty file with both reading and writing mode.
	//checking the existence of file through file pointer
	
	if(filePointer!= nullptr)
    {
    	SM_PageHandle new_page= (SM_PageHandle)malloc(PAGE_SIZE * sizeof(char)); 
        block_num++;
        setEmptyMemory(new_page);
	    // creating empty page using malloc
	    //checking if write operation is possible on the empty page.
	    (PAGE_SIZE >= (fwrite(new_page, sizeof(char), PAGE_SIZE, filePointer)))?printf("\nzero page has been appended to file pointed by filePointer"):printf("\nzero page cannot be appended to file");
	    
        free(new_page);// freeing the memory allocated for zero page
        block_num = block_num + 3;
	    fclose(filePointer);// closing the file to make sure buffers are flushed
	    printf("The new_page file was successfully closed");
        block_num--;
        ret_value = RC_OK;
	}
	else
	{   
        ++block_num;
		ret_value=RC_FILE_NOT_FOUND;
	}
	return ret_value;
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
--> Parameters:  File Handle : fHandle

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
--> parameters used: int pgeNum, SM_FileHandle *fHandle, SM_PageHandle memPage
--> return type: Return the success or failure operation using status code
-------------------------------------------------*/

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
--> Description: This function initiates access to the file and retrieves the initial block from the file
--> parameters used:SM_FileHandle *fileHandle, SM_PageHandle memPage

-------------------------------------------------*/




// Define the function to read the first block of a storage file.
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Attempt to open the specified file in read-only mode.
    // fHandle->fileName contains the path to the file.
    FILE *filePointer = fopen(fHandle->fileName, "r");
    // Check if the file opening was successful.
    if (!filePointer) {
        // If the file could not be opened, print an error message.
        printf("Unable to open file: %s\n", fHandle->fileName);
        // Return a predefined error code indicating the file was not found.
        return RC_FILE_NOT_FOUND;
    }

    // Attempt to read the first block (of size PAGE_SIZE) from the file into the memory buffer pointed by memPage.
    // fread reads elements of data, each of the size of a character, from the file.
    if (fread(memPage, sizeof(char), PAGE_SIZE, filePointer) < PAGE_SIZE) {
        // If fewer elements than PAGE_SIZE are read, it could be due to an error or end of file.
        // Check if we have reached the end of the file (EOF).
        if (feof(filePointer)) {
            // Print a message indicating EOF was reached before we could read PAGE_SIZE bytes.
            printf("Reached EOF before completing the read.\n");
        } else if (ferror(filePointer)) {
            // Check if there was an error during the read operation.
            printf("Error while reading the file.\n");
        }
        // Close the file to release system resources, as we are done with the file or encountered an error.
        fclose(filePointer);
        // Return a predefined error code indicating there was an error in reading.
        return RC_FILE_NOT_FOUND; // Assuming RC_READ_ERROR is a defined error code.
    }

    // If the read was successful, reset the current page position in the file handle to 0.
    // This indicates that the file pointer is at the beginning of the file after reading the first block.
    fHandle->curPagePos = 0;

    // Close the file to release system resources, as we have successfully completed the read operation.
    fclose(filePointer);
    // Return a success code indicating the operation completed successfully.
    return RC_OK;
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
--> Description:  This is used to read the next block in to memory address memPage
--> parameters used: SM_FileHandle *fHandle, SM_PageHandle memPage
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
 --> Author: Rashmi Venkatesh Topannavar
 --> Function Name: writeBlock
 --> Description: This function will write the data into the specified page number of the file.
 --> Parameters:  pageNum, SM_FileHandle *fHandle, SM_PageHandle mrPg
 --> Return type: Return Code
-------------------------------------------------*/
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle mrPg) {

    /* Write contents to the specified page. */

    int writeOffset = 0;
    int page_pos = 0;

    if (pageNum < 0 || pageNum > fHandle->totalNumPages) {
        page_pos = total_offset;
        return RC_WRITE_FAILED;
    }

    writeOffset = PAGE_SIZE * pageNum;
    block_num++;
    FILE *filePointer = fopen(fHandle->fileName, "r+");
    block_num = page_pos + 1;

    if (filePointer != nullptr) {
        if (pageNum != 0) {
            page_pos = 0;
            appendEmptyBlock(fHandle);
            fseek(filePointer, writeOffset, SEEK_SET);
            block_num ++;
            fwrite(mrPg, sizeof(char), PAGE_SIZE, filePointer); // Fixed the length parameter here.
            fHandle->curPagePos = ftell(filePointer);
            page_pos -= 1;
        }
        
        else {
            fseek(filePointer, writeOffset, SEEK_SET);
            block_num = block_num - total_offset;
            fwrite(mrPg, sizeof(char), PAGE_SIZE, filePointer);
            fHandle->curPagePos = ftell(filePointer);
            page_pos--;
        }

        fclose(filePointer);
        page_pos++;
        return RC_OK;
    }

    return RC_FILE_NOT_FOUND;
}

/*-----------------------------------------------
 --> Author: Uday Venkatesha
 --> Function Name: writeCurrentBlock
 --> Description: This function will write the data into the current block which the file handle is accessing
 --> Parameters : SM_FileHandle *fHandle, SM_PageHandle memPage
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



