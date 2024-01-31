***********************************************************************************************
CS 525 - Advanced Database Organisation
Assignment 1 - Storage Manager

*********************************       
      Group 11 Members:       
*********************************

Name - Suhas Palani
CWID - A20548277
Email- spalani3@hawk.iit.edu 


Name - Ramyashree Raghunandan 
CWID - A20541091
Email- rraghunandan@hawk.iit.edu


Name - Arpitha Hebri Ravi Vokuda    
CWID - A20541502 
Email- avokuda@hawk.iit.edu


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
Author: Arpitha Hebri Ravi Vokuda 		
Description: This function is used to read the pageNum block from the file defined by fHandle into address mrPg, parameters used: int pgeNum, SM_FileHandle *fileHandle, SM_PageHandle mrPg

Function Name: getBlockPos()
Author: Arpitha Hebri Ravi Vokuda
Description:  This function is used to return the current block position in file, parameters used: SM_FileHandle *fHandle

Function Name: readFirstBlock()
Author: Ramyashree Raghunandan
Description: This function opens the file in reads the first block of the file, parameters used:SM_FileHandle *fileHandle, SM_PageHandle mrPg

Function Name: readPreviousBlock()
Author: Ramyashree Raghunandan
Description: his function reads the previous block of the page using memory address mrPg, parameters used:char* fileName, SM_FileHandle *fHandle

Function Name: readCurrentBlock()
Author: Rashmi Venkatesh Topannavar
Description: This function reads the current block into the memory address mrPg, Parameters: SM_FileHandle *fHandle, SM_PageHandle mrPg

Function Name: readNextBlock()
Author: Arpitha Hebri Ravi Vokuda
Description: This is used to read the next block in to memory address mrPg, parameters used: SM_FileHandle *fHandle, SM_PageHandle mrPg

Function Name: readLastBlock()
Author: Ramyashree Raghunandan
Description: This function reads the last block of the page using memory address mrPg, parameters used:SM_FileHandle *fileHandle, SM_PageHandle mrPg


*********************************
    WRITE FUNCTIONS:
*********************************

Function Name: writeBlock()
Author: Rashmi Venkatesh Topannavar
Description: This function will write the data into the specified page number of the file, Parameters:  pageNum, SM_FileHandle *fHandle, SM_PageHandle mrPg

Function Name: writeCurrentBlock()
Author: Rashmi Venkatesh Topannavar
Description: This function will write the data into the current block which the file handle is accessing, Parameters : SM_FileHandle *fHandle, SM_PageHandle mrPg

Function Name: appendEmptyBlock()
Author: Ramyashree Raghunandan
Description: This function will append an empty page to the file, Parameters used are SM_FileHandle *fHandle

Function Name: ensureCapacity()
Author: Arpitha Hebri Ravi Vokuda
Description: This function make sure that the number of pages required, are available in the file. Parameters used are SM_FileHandle *fHandle and number of Pages

*********************************
    OTHER FUNCTIONS:
*********************************

Function Name: createPageFile()
Author: Arpitha Hebri Ravi Vokuda
Description: This function creates a new empty file and appending that empty file into the file pointed by filePointer, parameters used: fileName

Function Name: openPageFile()
Author: Ramyashree Raghunandan
Description: This function opens the file in read mode using file handle, parameters used: Filename and File Handle are the 2 parameters that are used

Function Name: closePageFile()
Author: Rashmi Venkatesh Topannavar
Description: This function closes the opened page file. Parameters:  File Handle

Function Name: destroyPageFile()
Author: Rashmi Venkatesh Topannavar
Description: This function deletes the page file, Parameters : File name

*********************************
    TEST CASES:
*********************************

Function Name:testCreateOpenClose()	
Description: It verifies if the code written, is able to create, open and close and delete a page file.

Function Name:testSinglePageContent()	
Description: It verified all the cases mentioned above along with which , it checks if the data can be read or written into the mentioned pageFiles.

*********************************************************************************************