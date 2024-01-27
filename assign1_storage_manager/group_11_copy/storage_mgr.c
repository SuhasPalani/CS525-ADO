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
-->Author: Arpitha Hebri Ravi Vokuda
--> Function: createPageFile()
--> Description: This function creates a new empty file and appending that empty file into the file pointed by filePointer
--> parameters used: fileName
--> return type: Return Code
-------------------------------------------------*/

extern RC createPageFile ( char * fileName )
{
    filePointer = fopen(fileName,"w+"); 
	
    //opening the file with write w+ mode, hence creating a empty file with both reading and writing mode.
	//checking the existence of file through file pointer
	
	if(filePointer!= nullptr)
    {
    	SM_PageHandle new_page= (SM_PageHandle)malloc(PAGE_SIZE * sizeof(char)); 
        setEmptyMemory(new_page);
	    // creating empty page using malloc
	    //checking if write operation is possible on the empty page.
	    (PAGE_SIZE >= (fwrite(new_page, sizeof(char), PAGE_SIZE, filePointer)))?printf("\nzero page has been appended to file pointed by filePointer"):printf("\nzero page cannot be appended to file");
	    
        free(new_page);// freeing the memory allocated for zero page
	    fclose(filePointer);// closing the file to make sure buffers are flushed
	    printf("The new_page file was successfully closed");
        ret_value = RC_OK;
	}
	else
	{
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

/*-------------------------------------------------
 --> Author: Rashmi Venkatesh Topannavar
 --> Function Name: closePageFile
 --> Description: This function closes the opened page file.
 --> Parameters:  File Handle
-------------------------------------------------*/

extern RC closePageFile(SM_FileHandle *fHandle)
{
	printf ("inside close\n"); 

    ret_value = (fHandle != 0) ? 
    ((filePointer = fopen(fHandle->fileName, "r")) != 0 && fclose(filePointer) == 0) ? RC_OK : RC_FILE_NOT_FOUND: RC_FILE_NOT_FOUND;

    return ret_value;
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
-->Author: Arpitha Hebri Ravi Vokuda
--> Function: readBlock()
--> Description: This function is used to read the pageNum block from the file defined by fHandle into address mrPg
--> parameters used: int pgeNum, SM_FileHandle *fileHandle, SM_PageHandle mrPg
--> return type: Return Code
-------------------------------------------------*/

RC readBlock (int pgNum, SM_FileHandle *fileHandle, SM_PageHandle mrPg)
		{
			if ((pgNum < fileHandle->totalNumPages)&& pgNum >= 0) 
			{
			//FILE *f; : used as a global variable
			//filePointer = fopen(fileHandle->fileName, "r"); //to open file
			 FILE *filePointer = fopen(fileHandle->fileName, "r");
			if(filePointer == nullptr){
					//If file doesn't exists it returns RC_FILE_NOT_FOUND
					return RC_FILE_NOT_FOUND; 
				}
			if (ferror(filePointer)){ //ferror() will detect error in file pointer stream	  
					printf("Error in reading from file...");
					clearerr(filePointer);      //clearerr() will clear error-indicators from the file stream
					ferror(filePointer);        //No error will be detected now
                    printf("Error detected in file pointer");
					return EXIT_FAILURE;
				}
			if(fseek(filePointer, pgNum * PAGE_SIZE, SEEK_SET) != 0) {
					fprintf(stderr, "fseek() failed in file storage_mgr \n");
						fclose(filePointer);
						return EXIT_FAILURE;
				}	
			if (fread(mrPg, sizeof(char), PAGE_SIZE, filePointer) != PAGE_SIZE) {
            fclose(filePointer);
            return EXIT_FAILURE;
        	}

        fclose(filePointer);
        fileHandle->curPagePos = pgNum;
        return RC_OK;
    } else {
    	//If the total number of pages is more than the page number it will throw error of Non Exisitng Page.
        return RC_READ_NON_EXISTING_PAGE;
    }
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
--> Author: Ramyashree Raghunandan
--> Function: readPreviousBlock()
--> Description: This function reads the previous block of the page using memory address mrPg
--> parameters used:char* fileName, SM_FileHandle *fHandle
--> return type: Return Code
-------------------------------------------------*/

RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle mrPg) {

    return (fHandle != nullptr && fHandle->curPagePos > 0)
        ? readBlock(fHandle->curPagePos - 1, fHandle, mrPg)
        : RC_FILE_NOT_FOUND;

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
