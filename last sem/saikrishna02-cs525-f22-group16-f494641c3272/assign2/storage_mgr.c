#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "dberror.h"

FILE *filePointer;
RC ret_value;

void initStorageManager (void)
{
    /*First call made toi initStorage*/
    filePointer = NULL;
    ret_value = -1;
    printf("Initializing the Storage Manager");
	printf("\nStorage Manager Initialized");
}


/**********************
 * Author: Yashas Shashidhar
 * Function Name: setNull
 * Description: This function will set the date to NULL.
 * Parameters used are fileName
*********************/
void setNull (char *memory){

    int i;
    for (i=0; i < PAGE_SIZE; i++){
        memory[i] = '\0';
    }
    return;
}

/**********************
 * Author: Bhoomika Panduranga
 * Function Name: createPageFile
 * Description: This function will create a fill and appends an single empty file.
 * Parameters used are fileName
*********************/
extern RC createPageFile ( char * fileName )
{
    filePointer = fopen(fileName,"w+"); 
	
    //opening the file with write w+ mode, hence creating a empty file with both reading and writing mode.
	//checking the existence of file through file pointer
    
    if (filePointer == NULL)
	    ret_value = RC_FILE_NOT_FOUND;

	else 
    {
        SM_PageHandle zeroPage= (SM_PageHandle)malloc(PAGE_SIZE * sizeof(char)); 
        setNull(zeroPage);
	    // creating empty page using calloc
	    //checking if write operation is possible on the empty page.
	    if(fwrite(zeroPage, sizeof(char), PAGE_SIZE, filePointer))
	    {
		    printf("zero page is written to file");
	    }
    	else
	    {
		    printf("zero page cannot be written to file");
	    }
	    
        free(zeroPage);// freeing the memory allocated for zero page
	    fclose(filePointer);// closing the file to make sure buffers are flushed
	    
        ret_value = RC_OK;
	}
	return ret_value;
}

/******************************************************************************
 * Aurthor: Bhoomika Panduranga
 * Function Name: openPageFile
 * Description: This function will create a file handel to the open file..
 * Parameters used are fileName and File Handle
******************************************************************************/

extern RC openPageFile(char* fileName, SM_FileHandle *fHandle)
{   
    int totalNumPages = 0;
	
    filePointer = fopen(fileName,"r+"); // opening file in r mode to create an empty page in read only

    if(filePointer != NULL)
    {
        fHandle->curPagePos = 0;// initializing current position to zero.
		fHandle->fileName = fileName;// initializing the filename of the file
		
        fseek(filePointer, 0, SEEK_END);// finding the end point of the file
		totalNumPages = ftell(filePointer) ;// calculating number of pages using the filepointer end location and page size given.
		fHandle->totalNumPages = totalNumPages / PAGE_SIZE;// initializing total number of pages
		fclose(filePointer);// closing the file

		return RC_OK;
    }
    return RC_FILE_NOT_FOUND;
	
	
    
}

/***********************************************************
 * Author: Bhoomika Panduranga
 * Function Name: closePageFile
 * Description: This function closes the opened page file.
 * Parameters used are File Handle
*************************************************************/

extern RC closePageFile(SM_FileHandle *fHandle)
{
    // if(filePointer != NULL)
	// 	filePointer = NULL;	
	// return RC_OK; 

	// using fclose function to close the file
	// printf ("inside close\n"); 

    if (fHandle != NULL)
    {
        filePointer = fopen(fHandle->fileName, "r");

        if (filePointer != NULL)
            if (fclose(filePointer) == 0) // checking if file exists 
                ret_value = RC_OK;
            
    }
    else 
        ret_value = RC_FILE_NOT_FOUND;

    return ret_value;
}

/***********************************************************
 * Author: Bhoomika Panduranga
 * Function Name: destroyPageFile
 * Description: This function delets the page file
 * Parameters used are file name
*************************************************************/

extern RC destroyPageFile(char *fileName)
{
	
    filePointer = fopen(fileName, "r");// opening the file in read mode to get the file pointer 
	
    if(filePointer != NULL)
    {
	    remove(fileName);// removing the file if the file is found
	    ret_value = RC_OK;
	}
	else
	    ret_value = RC_FILE_NOT_FOUND;	

	return ret_value;
}


/************************************************************************************************************
 * Author: Diksha Sharma
 * Function Name: readBlock
 * Description: This is used to read the pageNum block from the file defined by fHandle into address mrPg
 * Parameters Used are int pageNum, SM_FileHandle *fHandle, SM_PageHandle mrPg
*************************************************************************************************************/

extern RC readBlock (int pgNum, SM_FileHandle *fileHandle, SM_PageHandle mrPg)
		{
			if (pgNum < 0 || pgNum > (*fileHandle).totalNumPages ) 
				return RC_READ_NON_EXISTING_PAGE; //If the total number of pages is more than the page number it will throw error of Non Exisitng Page.
			
			//FILE *f; : used as a global variable in common.h
			
			filePointer = fopen(fileHandle->fileName, "r"); //open file

			if(filePointer == NULL)
				{  //If file doesn't exists
					return RC_FILE_NOT_FOUND; 
				}
			else
				{
					int offset = pgNum * PAGE_SIZE;
					if (fseek(filePointer, offset, SEEK_SET))
					{
						return EXIT_FAILURE;
					}
					fread(mrPg, sizeof(char), PAGE_SIZE, filePointer);
					fileHandle->curPagePos = ftell(filePointer); // Current page position update to pagenum 
				}
			fclose(filePointer);
			return RC_OK; 
		}
    		
/***********************************************************************************
 * Author: Diksha Sharma
 * Function Name: getBlockPos
 * Description: This function is used to return the current block position in file
 * Parameters used are SM_FileHandle *fHandle
************************************************************************************/

extern RC getBlockPos (SM_FileHandle *fHandle)
{
    /*Current Position of the pointer has to be returned */

    int curPos = 0;

	if(fHandle == NULL)                                                     
    {
       
       ret_value = RC_FILE_NOT_FOUND;
                                        
    }
    else
    {
        curPos = fHandle->curPagePos;
        ret_value = curPos;
    }	

    return ret_value;
}

/*********************************************************************
 * Author: Diksha Sharma
 * Function Name: readFirstBlock
 * Description: This function is used to read the first block of file.
 * Parameters used are M_FileHandle *fHandle, SM_PageHandle mrPg
**********************************************************************/


extern RC readFirstBlock (SM_FileHandle *fileHandle, SM_PageHandle mrPg)
{
    
    filePointer = fopen(fileHandle->fileName, "r");

    if(filePointer == NULL)
        {  //If file doesn't exists
            printf("file does not exist");
            return RC_FILE_NOT_FOUND; 
        }
    else if (ferror(filePointer)) //ferror() will detect error in file pointer stream
        {   
            printf("Error in reading from file!");
        
            clearerr(filePointer);      //clearerr() will clear error-indicators from the file stream
            ferror(filePointer);       //No error will be detected now
            printf("Error again in reading from file!");
            return EXIT_FAILURE;
        }
    else
        {
            if(fseek(filePointer, 0, SEEK_SET))
                {
                    fprintf(stderr, "fseek() failed in file %s at line # %d\n", __FILE__, __LINE__ - 2);
                    fclose(filePointer);
                    return EXIT_FAILURE;
                }
            fread(mrPg, sizeof(char), PAGE_SIZE, filePointer); //This return the first block of PAGE_SIZE
            if(feof(filePointer))
                {       
                    //On detecting the end-of-file, feof() function will return non-zero value
                    printf("Error reading: unexpected end of file\n");   
                    //hence, it will break the loop
                    return EXIT_FAILURE;
                }
            (*fileHandle).curPagePos = ftell(filePointer); //First page of the file has index 0
        }
    fclose(filePointer);
    //Successful exit from the program
    return RC_OK;
}

/***************************************************************************************************************
 * Author: Diksha Sharma
 * Function Name: readPreviousBlock
 * Description: This is used to read the previousblock in this file into the address which mrPg has pointed.
 * Parameters used are SM_FileHandle *fHandle, SM_PageHandle mrPg
****************************************************************************************************************/

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /* Using the present blocks poosition, we find the previous block and read it's comtents*/
	if(fHandle != NULL)                                                     
    {
	    return(readBlock (fHandle->curPagePos-1, fHandle, mrPg));                                   
    }
    else
    {
        return RC_FILE_NOT_FOUND; 
    }	
}

/***********************************************************************
 * Author: Diksha Sharma
 * Function Name: readCurrentBlock
 * Description: This reads the current block into memory address mrPg
 * Parameters: SM_FileHandle *fHandle, SM_PageHandle mrPg
************************************************************************/


extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /*Using the present blocks poosition, we read it's comtents*/
    if(fHandle != NULL)                                                     
    {
	    return(readBlock (fHandle->curPagePos, fHandle, mrPg));                               
    }
    else
    {
        return RC_FILE_NOT_FOUND; 
    }	

}


/**********************************************************************************************************************
 * Author: Diksha Sharma
 * Function Name: readNextBlock
 * Description: This is used to read the next block in to memory address mrPg
 * Parameters used are SM_FileHandle *fHandle, SM_PageHandle mrPg
***********************************************************************************************************************/


extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /*Using the present blocks poosition, we find the next block and read it's comtents*/
    if(fHandle != NULL)                                                     
    {
        return(readBlock (fHandle->curPagePos+1, fHandle, mrPg));                                 
    }
    else
    {
        return RC_FILE_NOT_FOUND; 
    }	        
}

/******************************************************************************************
 * Author: Diksha Sharma
 * Function Name: readLastBlock
 * Description: This is used to read the last block in this file into memory address mrPg
 * Parameters used are SM_FileHandle *fHandle, SM_PageHandle mrPg
********************************************************************************************/


extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /*Read contents of the last page in the block*/	
    if(fHandle != NULL)                                                     
    {
	    return(readBlock (fHandle->totalNumPages-1, fHandle, mrPg));                              
    }
    else
    {
        return RC_FILE_NOT_FOUND; 
    }	
}

/******************************************************************************************
 * Author: Yashas Shashidhar
 * Function Name: writeBlock
 * Description: This function will write the data into the specified page number of the file.
 * Parameters used are pageNum SM_FileHandle *fHandle, SM_PageHandle mrPg
********************************************************************************************/


extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle mrPg) {

    /*Write contents to into the specified page.*/

    int writeOffset = 0;

    if (pageNum >= 0 && pageNum <= fHandle->totalNumPages){
            writeOffset = PAGE_SIZE * pageNum;
            filePointer = fopen(fHandle->fileName, "r+");

            if (filePointer != NULL)
            {   //Initially the filepointer has to moved the mentioned page number.
                if(pageNum ==0){
                    fseek(filePointer, writeOffset, SEEK_SET);
                    fwrite(mrPg, sizeof(char), PAGE_SIZE, filePointer);
                    fHandle->curPagePos = ftell(filePointer);
                }
                else {
                    appendEmptyBlock(fHandle);
                    fseek(filePointer, writeOffset, SEEK_SET);
                    fwrite(mrPg, sizeof(char), strlen(mrPg), filePointer);
                    fHandle->curPagePos = ftell(filePointer);
                }

                fclose(filePointer);
                return  RC_OK;  
            }  
            fclose(filePointer);
            return  RC_FILE_NOT_FOUND;                         
        }
        
    return RC_WRITE_FAILED;
            	
    
    
}

/********************************************************************************************************
 * Author: Yashas Shashidhar
 * Function Name: writeCurrentBlock
 * Description: This function will write the data into the current block the file handle is accessing
 * Parameters used are SM_FileHandle *fHandle, SM_PageHandle mrPg
**********************************************************************************************************/


extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /* Writing the mrPg data into the file by passing the curPagePos to the writeblock */

    int curBlock = 0;

    if(fHandle == NULL)                                                     
    {
        return RC_FILE_NOT_FOUND; 
                                     
    }
    
    curBlock=fHandle->curPagePos;
	return(writeBlock (curBlock, fHandle, mrPg));  
    
        	
}

/********************************************************************************************************
 * Author: Yashas Shashidhar
 * Function Name: appendEmptyBlock
 * Description: This function will append an empty page to the file.
 * Parameters used are SM_FileHandle *fHandle,
**********************************************************************************************************/

extern RC appendEmptyBlock(SM_FileHandle *fHandle) 
{
    // To append an empty block the file poniter has to be seek to eof and 
    // new page has to added to the file. Later the total number of pages is increased by 1.

    
    if(fHandle != NULL)                                                     
    {
        // Creating an empty page

        SM_PageHandle emptyBlock = (SM_PageHandle)malloc(PAGE_SIZE*sizeof(char));
        setNull(emptyBlock); 

        //filePointer = fopen(fHandle->fileName, "w+");

        if(filePointer != NULL) 
        {
            // Moving file pointer to eof.

            fseek(filePointer, 0, SEEK_END);

            //writing the empty page into the file.

            if(fwrite(emptyBlock, 1, PAGE_SIZE, filePointer) == 0) 
            {   
                free(emptyBlock);            
             //   fclose(filePointer);
                return RC_WRITE_FAILED;                               
            }
            else
            {
                fHandle->totalNumPages++; //Incrementing the total number of pages by 1.       
                //fHandle->curPagePos = fHandle->totalNumPages - 1;       
                free(emptyBlock);   
                //fclose(filePointer);                                        
                return RC_OK;         
            } 

        } 
        //fclose(filePointer);
                               
    }
    
    return RC_FILE_NOT_FOUND; 
    	
}

/********************************************************************************************************
 * Author: Yashas Shashidhar
 * Function Name: ensureCapacity
 * Description: This function make sure that the number of pages required, are available in the file.
 * Parameters used are SM_FileHandle *fHandle and number of Pages
**********************************************************************************************************/

extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) 
{
    
    
    filePointer = fopen(fHandle->fileName, "a");
    int pgs;
    if (fHandle != NULL){

        pgs = numberOfPages - fHandle->totalNumPages;
        
        while(pgs > 0){
            appendEmptyBlock(fHandle);
            pgs--;
        }

        fclose(filePointer);
         ret_value = RC_OK;   
	}
    else
    {
                                       
        ret_value = RC_FILE_HANDLE_NOT_INIT;
    }

    return ret_value;
}