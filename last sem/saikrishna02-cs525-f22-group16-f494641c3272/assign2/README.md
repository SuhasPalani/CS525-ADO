CS 525 Assignment 2 - Buffer Manager

--------------------------------       
        Group 16 Members:
--------------------------------

Name - Diksha Sharma 
CWID - A20523396 
Email- dsharma19@hawk.iit.edu 


Name - Bhoomika Panduranga 
CWID - A20503493 
Email- bpanduranga@hawk.iit.edu


Name - Yashas Shashidhar    
CWID - A20516827 
Email- yshashidhar@hawk.iit.edu


--------------------------------
   Steps to run the code :
--------------------------------
$ make clean
This will delete all the previous complied files

$ make 
This will compile all project files and creates an executable file "test_assign1". Use the below command to run the executable.

$ ./test_assign1 or make run_test
This will execute the project.


If the code needs to be run again, we can clean the files and then Redo from Step 1. To clean use below command.

$ make clean



===========================
 We have implemented FIFO (First In First Out), LRU (Least Recently Used), LFU (Least Frequently Used) and CLOCK page replacement algorithms.

A. BUFFER POOL FUNCTIONS -Author: Diksha Sharma
===========================

The buffer pool related functions are used to create a buffer pool for an existing page file on disk. The buffer pool is created in memory while the page file is present on disk. We make the use of Storage Manager (Assignment 1) to perform operations on page file on disk.

1. initBufferPool
- This function creates a new buffer pool in memory.
- The parameter numPages defines the size of the buffer i.e. number of page frames that can be stored in the buffer.
- pageFileName stores the name of the page file whose pages are being cached in memory.
- strategy represents the page replacement strategy (FIFO, LRU, LFU, CLOCK) that will be used by this buffer pool
- stratData is used to pass parameters if any to the page replacement strategy. 

2. shutdownBufferPool
- This function shutsdown i.e. destroys the buffer pool.
- It free up all resources/memory space being used by the Buffer Manager for the buffer pool.
- Before destroying the buffer pool, we call forceFlushPool(...) which writes all the dirty pages (modified pages) to the disk.
- If any page is being used by any client, then it throws RC_PINNED_PAGES_IN_BUFFER error.

3. forceFlushPool
- This function writes all the dirty pages (modified pages whose dirtyBit = 1) to the disk.
- It checks all the page frames in buffer pool and checks if it's dirtyBit = 1 (which indicates that content of the page frame has been modified by some client) and fixCount = 0 (which indicates no user is using that page Frame) and if both conditions are satisfied then it writes the page frame to the page file on disk.


B. PAGE MANAGEMENT FUNCTIONS -Author: Bhoomika Panduranga, Yashas Shashidhar, Diksha Sharma
==========================
The page management functions are used to perform functions like unpin pages, load page from disk, set modified bit to 1 if page is modified and to write page frame to disk.

1. unpinPage
- In this function we will check if the task on the pin is done if yes then we will unpin the page.

2. forcePage 
- This function writes the contents of the modified pages back to the page file on disk.
- Iterating through all the pages in the buffer pool this function will check if the current page is the page which needs to be written
- we will set back the modified count to 0 after the modified page is updated to the disk.

3. pinPage
- This function is used to pin the page by taking the page file on disk. 
- we will ensure if the buferpool is empty before pinning the page.
- We call page replacement algorithms if there is no empty space in the page frame.
- If modified is set to one then the page will be replaced by new page as decided by page replacement algorithms.

4. makeDirty
- This function will set the modified bit to 1 if the page is modified.

C. STATISTICS FUNCTIONS -Author: Yashas Shashidhar
===========================
The statistics functions are used to get the information on buffer pool. It provides statistical data on buffer pools.

1. getFrameContents
- In this function we iterate through all the pages to get the pageId value of the page frame.
- we store the frame contents if pageId is found and will return the same.

2. getDirtyFlags
- This function will return the array containing dirty flags information.
- if the page is modified then set to true else false will be updated in the array.

3. getFixCounts
- This function will return an array containing the fix count of the page frame.
- To get the fix count it is neccessary to iterate over all the page frames.

4. getNumReadIO
- This function returns the number of pages read from the disk.
- page_read is used to get the information on pages read.

5. getNumWriteIO
- This function returns the count of number of pages written to the disk.
- num_write is used to get the number of count of writes.

6. copyPageFrames()
- This functions will copy the contents from a srource page to destination page

7. writePageFramse()
- This function write content from a page to pageframe

D. PAGE REPLACEMENT ALGORITHM FUNCTIONS -Author: Bhoomika Panduranga
=========================================

The page replacement strategy functions implement FIFO, LRU, LFU, CLOCK algorithms which are used while pinning a page. If the buffer pool is full and a new page has to be pinned, then a page should be replaced from the buffer pool. These page replacement strategies determine which page has to be replaced from the buffer pool.

FIFO(...)
--> First In First Out (FIFO) is the most basic page replacement strategy used.
--> FIFO is generally like a queue where the page which comes first in the buffer pool is in front and that page will be replaced first if the buffer pool is full.
--> Once the page is located, we write the content of the page frame to the page file on disk and then add the new page at that location.

LFU(...)
--> Least Frequently Used (LFU) removes the page frame which is used the least times (lowest number of times) amongst the other page frames in the buffer pool.
--> The variable (field) refNum in each page frame serves this purpose. refNum keeps a count of of the page frames being accessed by the client.
--> So when we are using LFU, we just need to find the position of the page frame having the lowest value of refNum.
--> We then write the content of the page frame to the page file on disk and then add the new page at that location.
--> Also, we store the position of the least frequently used page frame in a variable "lfuPointer" so that is useful next time when we are replacing a page in the buffer pool. It reduces the number of iterations from 2nd page replacement onwards.

LRU(...)
--> Least Recently Used (LRU) removes the page frame which hasn't been used for a long time (least recent) amongst the other page frames in the buffer pool.
--> The variable (field) hitNum in each page frame serves this purpose. hitNum keeps a count of of the page frames being accessed and pinned by the client. Also a global variable "hit" is used for this purpose.
--> So when we are using LRU, we just need to find the position of the page frame having the lowest value of hitNum.
--> We then write the content of the page frame to the page file on disk and then add the new page at that location.

CLOCK(...)
--> CLOCK algorithm keeps a track of the last added page frame in the buffer pool. Also, we use a clockPointer which is a counter to point the page frames in the buffer pool.
--> When a page has to be replaced we check the "clockPointer"s position. If that position's page's hitNum is not 1 (i.e. it wasn't the last page added), then replace that page with the new page.
--> In case, hitNum = 1, then we set it's hitNum = 0, increment clockPointer i.e. we go to the next page frame to check the same thing. This process goes on until we find a position to replace the page. We set hitNum = 0 so that we don't enter into an infinite loop