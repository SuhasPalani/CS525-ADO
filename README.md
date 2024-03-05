***********************************************************************************************
CS 525 - Advanced Database Organisation
Assignment 1 - Storage Manager

*********************************       
      Group 11 Members:       
*********************************

<<<<<<< HEAD
Name - Suhas Palani
CWID - A20548277
Email- spalani3@hawk.iit.edu 
=======
Spring 2024- Illinois Institute of Technology

>>>>>>> c7dc14127dfd5f716980d174edd7104f93cc8845


Name - Nishchal Gante Ravish 
CWID - A20540635
Email- nganteravish@hawk.iit.edu


Name - Uday venkatesha 
CWID - A20547055
Email- uvenkatesha@hawk.iit.edu


*********************************
   Steps to run the code :
*********************************
The below commands should be executed in the terminal inorder to run the code.

Step 1:
Command: $ make

Description: This creates an executable file "test_assign1". 

Step 2:
Command: $ ./test_assign1

Description: This command executes the excecutable file which is newly created.

Step 3:
Command: $ valgrind --leak-check=full --track-origins=yes ./test_assign1

Description: Once the code is executed, the above command is used to monitor the data leak.

Note:
If it's necessary to execute the code again, we should clear the files and then restart the process.

Command: $ make clean

Description: To clear the data use the above command.

*********************************
    READ FUNCTIONS:
*********************************

Function Name: readBlock()
Author: Nishchal Gante Ravish 		
Description: This function reads the pageNum block from the file defined by fHandle into address memPage and thr parameters used by this function are int pageNum, SM_FileHandle *fileHandle, and SM_PageHandle memPage.

Function Name: getBlockPos()
Author: Suhas Palani
Description:  Returning the current block location in the file is the purpose of this method.Parameters used: SM_FileHandle *fHandle

Function Name: readFirstBlock()
Author: Uday Venkatesha
Description: The parameters used by this function are as follows: It opens the file and reads its first block.SM_FileHandle *fileHandle, SM_PageHandle memPage

Function Name: readPreviousBlock()
Author: Nishchal Gante Ravish
Description: Using the memory address memPage, this function is used to read the previous block of the page and the params involved are SM_FileHandle *fHandle, char* fileName

Function Name: readCurrentBlock()
Author: Suhas Palani
Description: The current block is read by this function and stored in the memory location memPage.
Parameters: SM_FileHandle *fHandle, SM_PageHandle memPage

Function Name: readNextBlock()
Author: Uday Venkatesha
Description: The purpose of this is to read the subsequent block from memory address memPage.
parameters used: SM_FileHandle *fHandle, SM_PageHandle memPage

Function Name: readLastBlock()
Author: Nishchal Gante Ravish
Description: Used to read the last block of the page using memory address. The parameters used here are SM_FileHandle *fileHandle, SM_PageHandle memPage


*********************************
    WRITE FUNCTIONS:
*********************************

Function Name: writeBlock()
Author: Suhas Palani
Description: This function will enter the data into the file on the designated page. Parameters:  pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage

Function Name: writeCurrentBlock()
Author: Uday Venkatesha
Description: The data will be written by this function into the block that the file handle is now accessing, Parameters : SM_FileHandle *fHandle, SM_PageHandle memPage

Function Name: appendEmptyBlock()
Author: Nishchal Gante Ravish
Description: This given function is used to append an empty page to the file.
Parameters used: SM_FileHandle *fHandle

Function Name: ensureCapacity()
Author: Suhas Palani
Description: This function verifies that the file contains the necessary number of pages. Parameters used are SM_FileHandle *fHandle and number of Pages

*********************************
    OTHER FUNCTIONS:
*********************************

Function Name: createPageFile()
Author: Uday Venkatesha
Description: Using filePointer as a pointer, this function appends a new, empty file to the existing one, parameters used: fileName


Function Name: openPageFile()
Author: Nishchal Gante Ravish
Description: Using the file handle and the Filename and File Handle parameters, this function is used to open the file in read mode

Function Name: closePageFile()
Author: Suhas Palani
Description: The opened page file is closed using this method. Parameters Used:  File Handle :fHandle

Function Name: destroyPageFile()
Author: Uday Venkatesha
Description: This function is used to delete the page file, Parameters : File name

*********************************
    TEST CASES:
*********************************

Function Name:testCreateOpenClose()	
Description: It checks to see if the written code can create, open, shut, and remove page files.


Function Name:testSinglePageContent()	
Description: Along with verifying each of the aforementioned scenarios, it also checked to see if the data could be written to or read from the specified page files.


*********************************************************************************************
