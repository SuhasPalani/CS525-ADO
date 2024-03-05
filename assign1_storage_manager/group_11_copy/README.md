***********************************************************************************************
CS 525 - Advanced Database Organisation
Assignment 1 - Storage Manager
***********************************************************************************************

*********************************       
Group 11 Members:       
*********************************

- Name: Suhas Palani
  CWID: A20548277
  Email: spalani3@hawk.iit.edu 

- Name: Nishchal Gante Ravish 
  CWID: A20540635
  Email: nganteravish@hawk.iit.edu

- Name: Uday Venkatesha 
  CWID: A20547055
  Email: uvenkatesha@hawk.iit.edu

*********************************
Steps to Run the Code:
*********************************

1. Compile the Code:
   - Command: make
   - Description: This creates an executable file "test_assign1". 

2. Execute the Program:
   - Command: ./test_assign1
   - Description: This command executes the executable file which is newly created.

3. Monitor for Data Leaks:
   - Command: valgrind --leak-check=full --track-origins=yes ./test_assign1
   - Description: Once the code is executed, use the above command to monitor the data leak.

Note: To execute the code again, clear the files and restart the process.
   - Command: make clean
   - Description: To clear the data use the above command.

*********************************
Read Functions:
*********************************

- Function Name: readBlock()
  - Author: Nishchal Gante Ravish
  - Description: This function reads the pageNum block from the file defined by fHandle into address memPage. 
  - Parameters: int pageNum, SM_FileHandle *fileHandle, SM_PageHandle memPage

- Function Name: getBlockPos()
  - Author: Suhas Palani
  - Description: Returning the current block location in the file.
  - Parameters: SM_FileHandle *fHandle

- Function Name: readFirstBlock()
  - Author: Uday Venkatesha
  - Description: It opens the file and reads its first block.
  - Parameters: SM_FileHandle *fileHandle, SM_PageHandle memPage

- Function Name: readPreviousBlock()
  - Author: Nishchal Gante Ravish
  - Description: This function is used to read the previous block of the page.
  - Parameters: SM_FileHandle *fHandle, SM_PageHandle memPage

- Function Name: readCurrentBlock()
  - Author: Suhas Palani
  - Description: The current block is read by this function.
  - Parameters: SM_FileHandle *fHandle, SM_PageHandle memPage

- Function Name: readNextBlock()
  - Author: Uday Venkatesha
  - Description: This is to read the subsequent block.
  - Parameters: SM_FileHandle *fHandle, SM_PageHandle memPage

- Function Name: readLastBlock()
  - Author: Nishchal Gante Ravish
  - Description: Used to read the last block of the page.
  - Parameters: SM_FileHandle *fileHandle, SM_PageHandle memPage

*********************************
Write Functions:
*********************************

- Function Name: writeBlock()
  - Author: Suhas Palani
  - Description: This function will enter the data into the file on the designated page.
  - Parameters: pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage

- Function Name: writeCurrentBlock()
  - Author: Uday Venkatesha
  - Description: The data will be written into the block that the file handle is currently accessing.
  - Parameters: SM_FileHandle *fHandle, SM_PageHandle memPage

- Function Name: appendEmptyBlock()
  - Author: Nishchal Gante Ravish
  - Description: This function is used to append an empty page to the file.
  - Parameters: SM_FileHandle *fHandle

- Function Name: ensureCapacity()
  - Author: Suhas Palani
  - Description: This function ensures that the file has the necessary number of pages.
  - Parameters: SM_FileHandle *fHandle, number of Pages

*********************************
Other Functions:
*********************************

- Function Name: createPageFile()
  - Author: Uday Venkatesha
  - Description: This function creates a new page file.
  - Parameters: fileName

- Function Name: openPageFile()
  - Author: Nishchal Gante Ravish
  - Description: This function opens the file in read mode.
  - Parameters: fileName, SM_FileHandle *fHandle

- Function Name: closePageFile()
  - Author: Suhas Palani
  - Description: The opened page file is closed.
  - Parameters: SM_FileHandle *fHandle

- Function Name: destroyPageFile()
  - Author: Uday Venkatesha
  - Description: This function deletes the page file.
  - Parameters: fileName

*********************************
Test Cases:
*********************************

- Function Name: testCreateOpenClose()
  - Description: Tests the ability to create, open, close, and destroy page files.

- Function Name: testSinglePageContent()
  - Description: Tests the functionality of writing to and reading from a single page file.

***********************************************************************************************
