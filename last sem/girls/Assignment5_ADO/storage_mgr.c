#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "dberror.h"

#define nullptr NULL

FILE *filePointer;
RC ret_value;

int block_num = 0;
int total_offset = 1;


/*-----------------------------------------------
 --> Author: Arpitha Hebri Ravi Vokuda, Ramyashree Raghunandan, Rashmi Venkatesh Topannavar
 --> Function Name: setEmptyMemory
 --> Description: This function will set the memory to NULL.
 --> Parameters used is Memory
-----------------------------------------------*/

void setEmptyMemory(char *memory) {
    size_t i = 0;
    do {
        memory[i] = '\0';
        i++;
        block_num = block_num + 1;
    } while (i < PAGE_SIZE);
}

/*-----------------------------------------------
-->Author: Arpitha Hebri Ravi Vokuda, Ramyashree Raghunandan, Rashmi Venkatesh Topannavar
--> Function: initStorageManager()
--> Description: This function initialises file pointer to null, return_code to -1. 
--> parameters used: void (no return value and no parameters)
-------------------------------------------------*/

void initStorageManager (void)
{
    /*First call made to initStorageManager*/
    //setting the filepointer to null
    filePointer = nullptr;
    block_num = 1;
    //setting the return_code value to null
    ret_value = -1;

    block_num ++;

    printf("\nDefining the Storage Manager function");
	printf("\nStorage Manager Defined and successfully initiallized");
    
}

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
--> Author: Ramyashree Raghunandan
--> Function: openPageFile()
--> Description: This function opens the file in read mode using file handle
--> parameters used: Filename and File Handle are the 2 parameters that are used
-------------------------------------------------*/


extern RC openPageFile(char* fileName, SM_FileHandle *fHandle) {
    // Opening the file in read mode
    FILE *filePointer = fopen(fileName, "r+");
    --block_num;

    return (filePointer != nullptr) ? (
        (fHandle->curPagePos = 0, fHandle->fileName = fileName, fseek(filePointer, 0, SEEK_END), fHandle->totalNumPages = (ftell(filePointer) + 1) / PAGE_SIZE, fclose(filePointer), RC_OK)
    ) : RC_FILE_NOT_FOUND;

}


/*-------------------------------------------------
 --> Author: Rashmi Venkatesh Topannavar
 --> Function Name: closePageFile
 --> Description: This function closes the opened page file.
 --> Parameters:  File Handle
-------------------------------------------------*/

extern RC closePageFile(SM_FileHandle *fHandle)
{
	printf ("inside close\n"); 
    block_num = 0;

    ret_value = (fHandle != 0) ? 
    ((filePointer = fopen(fHandle->fileName, "r")) != 0 && fclose(filePointer) == 0) ? RC_OK : RC_FILE_NOT_FOUND: RC_FILE_NOT_FOUND;
    block_num += 1;
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
    char page_file = 'O';

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
            total_offset = 0;
			if ((pgNum < fileHandle->totalNumPages)&& pgNum >= 0) 
			{
			//FILE *f; : used as a global variable
			//filePointer = fopen(fileHandle->fileName, "r"); //to open file
			 FILE *filePointer = fopen(fileHandle->fileName, "r");
             total_offset = total_offset +2;
			if(filePointer == nullptr){
					//If file doesn't exists it returns RC_FILE_NOT_FOUND
					return RC_FILE_NOT_FOUND; 
				}
			if (ferror(filePointer)){ //ferror() will detect error in file pointer stream	
                    total_offset = total_offset-1;  
					printf("Error in reading from file...");

					clearerr(filePointer);      //clearerr() will clear error-indicators from the file stream
					--total_offset;
                    ferror(filePointer);        //No error will be detected now
                    -
                    printf("Error detected in file pointer");
					return EXIT_FAILURE;
				}
			if(fseek(filePointer, pgNum * PAGE_SIZE, SEEK_SET) != 0) {
                total_offset++;
					fprintf(stderr, "fseek() failed in file storage_mgr \n");
						fclose(filePointer);
                        total_offset = total_offset-1;
						return EXIT_FAILURE;
				}	
			if (fread(mrPg, sizeof(char), PAGE_SIZE, filePointer) != PAGE_SIZE) {
            fclose(filePointer);
            total_offset--;

            return EXIT_FAILURE;
        	}

        fclose(filePointer);
        fileHandle->curPagePos = pgNum;
        total_offset--;
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
            total_offset = total_offset + 1;
            block_num = 0;

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
    total_offset = block_num + 1;

    RC status = (filePointer == nullptr) ? RC_FILE_NOT_FOUND : RC_OK;
    //Checking if file opened successfully
    if (status == RC_OK) {
        (fseek(filePointer, 0, SEEK_SET)) ? (fprintf(stderr, "fseek() failed in file storage_mgr.c\n"), fclose(filePointer), EXIT_FAILURE) : 0;
        total_offset = block_num - 1;
        fread(mrPg, sizeof(char), PAGE_SIZE, filePointer);
        

        if (feof(filePointer)) {
            printf("Error reading: unexpected end of file\n");
            fclose(filePointer);
            total_offset = block_num ;

            return EXIT_FAILURE;
        }
        fileHandle->curPagePos = 0;
    }
    if (filePointer != nullptr) {
        block_num --;
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

    block_num = total_offset;
    return (fHandle != nullptr && fHandle->curPagePos > 0)? readBlock(fHandle->curPagePos - 1, fHandle, mrPg) : RC_FILE_NOT_FOUND;

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
    total_offset = block_num ;
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
    int next_Blk = 1;
    
switch (1) {
    case 1:
        switch (fHandle != nullptr) {
            case 1:
                return readBlock(fHandle->curPagePos + 1, fHandle, mrPg);
            case 0:
                return RC_FILE_NOT_FOUND;
                next_Blk--;
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
    int total_Blk = 0;

    return (fHandle != nullptr)
        ? readBlock((fHandle->totalNumPages > 0) ? (fHandle->totalNumPages - 1) : 0, fHandle, mrPg)
        : RC_FILE_NOT_FOUND;
        total_Blk--;
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
 --> Author: Rashmi Venkatesh Topannavar
 --> Function Name: writeCurrentBlock
 --> Parameters : SM_FileHandle *fHandle, SM_PageHandle mrPg
 --> Description:  The writeCurrentBlock function writes data which the file handle is accessing into the current block.
 --> Return type: Return Code
-----------------------------------------------*/


RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /* Writing the mrPg data into the file by passing the curPagePos to the writeblock */
    int block_num = 1;

    int current_Block = (fHandle == 0) ? RC_FILE_NOT_FOUND : fHandle->curPagePos;
    block_num = block_num+1;
    return writeBlock(current_Block, fHandle, mrPg);
    block_num++;
        	
}

/*-------------------------------------------------
 --> Author: Ramyashree Raghunandan
 --> Function Name: appendEmptyBlock
 --> Description: This function will append an empty page to the file.
 --> Parameters used are SM_FileHandle *fHandle
---------------------------------------------------*/

extern RC appendEmptyBlock(SM_FileHandle *fHandle) {
    // To append an empty block, the file pointer has to be moved to the end of the file,
    // and a new page needs to be added to the file. Then, the total number of pages is increased by 1.
    int fileNum = 0;
    if (fHandle != nullptr) {
        // Create an empty page.
        SM_PageHandle emptyBlock = (SM_PageHandle)malloc(PAGE_SIZE * sizeof(char));
        block_num = fileNum;
        setEmptyMemory(emptyBlock);

        FILE *filePointer = fopen(fHandle->fileName, "ab");
        fileNum = -1;

        if (filePointer != nullptr) {
            // Move the file pointer to the end of the file.
            fseek(filePointer, 0, SEEK_END);
            total_offset = fileNum + block_num;

            // Write the empty page to the file.
            if (fwrite(emptyBlock, 1, PAGE_SIZE, filePointer) == PAGE_SIZE) {
                fHandle->totalNumPages++; // Increment the total number of pages by 1.
                fileNum = 0;
                free(emptyBlock);
                fclose(filePointer);
                total_offset = fileNum + 1;
                return RC_OK;
            } else {
                fileNum++;
                free(emptyBlock);
                fclose(filePointer);
                total_offset += 1;
                return RC_WRITE_FAILED;
                block_num++;
            }
        } else {
            free(emptyBlock);
            total_offset = 1;
            return RC_FILE_NOT_FOUND;
            fileNum -- ;
        }
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
    int capacity = 1;
    int i;
    
switch (1) {
        case 1:
            if (fHandle != nullptr) {
                capacity = -1;
                No_of_pg = numberOfPages - (*fHandle).totalNumPages;


                switch (No_of_pg > 0) {
                    case 1:
                    	//if the number of pages is greater than total number of pages 
						i = 0;
   						loop_start:
    					if (i < No_of_pg) {
        				appendEmptyBlock(fHandle);
        				i++;
                        capacity++;
        				goto loop_start;
    					}
    					ret_value = RC_OK;
                        break;

                    case 0:
                        // If the number of pages is within the total number of pages.
                        ret_value = RC_WRITE_FAILED;  // Changed error code
                        capacity = -1;
                        break;
                }
            } else {
                ret_value = RC_FILE_HANDLE_NOT_INIT;  // Changed error code
                capacity++;
            }
            break;
    }

    return ret_value;
}
