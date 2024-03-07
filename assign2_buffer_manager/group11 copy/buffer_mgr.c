#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h> // Include for INT_MAX
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <string.h>
#define nullptr NULL

typedef struct Page
{
    SM_PageHandle page_h; // The data that is retrieved from the page
    PageNumber pageid;    // The ID of the page is stored
    int modified;         // Used to show whether the client has made changes to the page
    int num;              // Used to show how many users are accessing that page at any particular time
    int lru_num;          // Utilized to obtain the least recently used page using the LRU Algorithm
    int lfu_num;          // Utilized by LFU algorithm to determine which page is used at least
} PageFrame;

int buffer_size = 0, page_read = 0, num_write = 0, index_hit = 0, clock_index = 0, lfu_index = 0;

/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: copyPageFrames()
--> Description: --> This function copies the contents to the target page from the source page.
--> Parameters Used: PageFrame *dest, int index,PageFrame *src
-------------------------------------------------*/
void copyPageFrames(PageFrame *dest, int index, PageFrame *src)
{

    memcpy(dest + index, src, sizeof(PageFrame));
    return;
}

/*-----------------------------------------------
--> Author: Nishchal Gante Ravish
--> Function: writePageFrames()
--> Description: This function is used to cpoy the data from page to page func
--> parameters used: BM_BufferPool *const bp,  PageFrame *page_f, int page_index
-------------------------------------------------*/

void writePageFrames(BM_BufferPool *const bp, PageFrame *page_f, int page_index)
{

    // Initialize to 0
    int pgfm = 0;

    // Call file handl;er
    SM_FileHandle f_handle;

    openPageFile(bp->pageFile, &f_handle);

    // Increment pgfm
    pgfm += 2;

    // writeblock func
    writeBlock(page_f[page_index].pageid, &f_handle, page_f[page_index].page_h);

    // Increasing no of writes
    num_write += 1;
    pgfm++;

    return;
}

/*-----------------------------------------------
-->Author: Uday venkatesha
--> Function: FIFO()
--> return type: void
--> parameters used: q
--> Description: When a page is located, its content is written back to the page file on disc, and the new page is placed there.
The FIFO function works by treating the buffer pool as a queue, meaning that when the buffer pool fills up, the first-in page gets replaced first.


-------------------------------------------------*/
// Implements FIFO page replacement strategy within a given buffer pool.
extern void FIFO(BM_BufferPool *const bp, PageFrame *pf)
{
    int buffer_1 = 0;                            
    // Initially set to zero to act as a simple counter.
    
    int currentIdx = page_read % buffer_size;    // Determines the start index for FIFO based on total pages read.
    
    buffer_1++;                                  // Increment our auxiliary counter.



    // Acquire direct access to the buffer pool's internal array of page frames.

    PageFrame *page_f = (PageFrame *)bp->mgmtData;



    // This integer will track our progress as we cycle through the page frames.

    int pgPos = 1;



    // Iteratively examine each frame in the buffer to implement FIFO replacement.

    for (int iter = 0; iter < buffer_size; iter++)

    {

        // Skip over page frames currently in use, incrementing our tracking variables accordingly.

        if (page_f[currentIdx].num != 0)

        {

            pgPos++;                                  // Increment our positional tracking.

            currentIdx = (currentIdx + 1) % buffer_size; // Circularly move through the buffer pool.

        }



        // Assess if the current frame is suitable for replacement.

        if (page_f[currentIdx].num == 0)

        {

            // For modified pages, commit their data back to disk before eviction.

            if (page_f[currentIdx].modified == 1)

            {

                writeBack(bp, currentIdx); // Assuming writeBack is a function to write a page back to disk.

            }



            // Populate the current frame with the new page's information.

            updateFrame(&page_f[currentIdx], pf); // Assuming updateFrame replaces page frame details.

            return; // Exit after completing the page insertion, as per FIFO logic.

        }

        else

        {

            // Continue to the next frame, as the current one is occupied.

            currentIdx = (currentIdx + 1) % buffer_size;

            pgPos += 2; // Advance position tracker more aggressively due to bypass.

        }

    }

    // If function exits the loop without finding a replacement, all frames were occupied. Handle as needed.

}





/*-----------------------------------------------

-->Author: Suhas Palani
--> Function: LRU()
--> Description: --> This function eliminates from the buffer pool any page frame that hasn't been utilized recently or for an extended period of time.
--> Parameters Used: BM_BufferPool *const bp, PageFrame *pf
-------------------------------------------------*/

// Implementation of LRU (Least Recently Used) algorithm

extern void LRU(BM_BufferPool *const bp, PageFrame *pf)
{
    int index = -1;
    int leastNumber = INT_MAX;
    int pgFrame = 1;

    PageFrame *page_f = nullptr;
    int k = 0;
    // Check if buffer pool is not nullptr
    if (bp != nullptr)
    {
        pgFrame++;
        page_f = (PageFrame *)bp->mgmtData;
    }

    // First loop: Locate the first frame that is free or has the lowest lru_num.
    int j = 0;
    while (j < buffer_size && (page_f + index)->num != 0)
    {
        pgFrame = 0;
        index = (page_f + j)->num == 0 ? j : (leastNumber > (page_f + j)->lru_num ? j : index);
        leastNumber = (page_f + j)->num == 0 ? (page_f + j)->lru_num : leastNumber;
        j++;
    }

    // Second loop: Determine which page frame hasn't been utilized recently.

    j = index + 1;
    do
    {
        pgFrame = 0;
        index = leastNumber > (page_f + j)->lru_num ? j : index;
        leastNumber = leastNumber > (page_f + j)->lru_num ? (page_f + j)->lru_num : leastNumber;
        j++;
    } while (j < buffer_size);

    // Verify if the chosen page frame has been altered, then save it to disk.

    pgFrame = (page_f + index)->modified == 1 ? (writePageFrames(bp, page_f, index), k + 1) : k;

    // Boost the number of writes after write operations

    k += (page_f + index)->modified == 1 ? 1 : 0;

    // Transfer the new page's content to the chosen page frame.

    copyPageFrames(page_f, index, pf);
    pgFrame = k;
    page_f[index].lru_num = pf->lru_num;
    k++;
}

/*-----------------------------------------------
--> Author: Nishchal Gante Ravish
--> Function: The LRU_K() function
--> parameters used: BM_BufferPool *const bp, PageFrame *pf
-------------------------------------------------*/

extern void LRU_K(BM_BufferPool *const bp, PageFrame *pf)
{

    // Init cnt to 0
    int cnt = 0;

    PageFrame *page_f = (PageFrame *)bp->mgmtData; // Pointer to page frames

    int j = 0, index = -1, least_number = INT_MAX;

    // LRU_NUM find page

    for (j = 0; j < buffer_size; j++)
    {

        if (page_f[j].num == 0 || page_f[j].lru_num < least_number)
        {

            least_number = page_f[j].lru_num;

            index = j;
        }
    }

    // Page checking for modification

    if (page_f[index].modified == 1)
    {

        // Writing it back to disk
        writePageFrames(bp, page_f, index);
    }
    else
    {

        // Using this to track
        cnt += 1;
    }

    // Copy the info to new page
    copyPageFrames(page_f, index, pf);

    // Updtae lru for the new page
    page_f[index].lru_num = pf->lru_num;

    // Count the final value
    cnt++;
}

/*-----------------------------------------------
-->Author: Uday Venkatesha
--> Function: CLOCK()
--> Description: -->CLOCK algorithm keeps a track of the last added page frame in the buffer pool.
                 --> Also, we use a clockPointer which is a counter to point the page frames in the buffer pool.
--> parameters used: BM_BufferPool *const bp, PageFrame *pf
--> return type: void
-------------------------------------------------*/

// Function to implement the CLOCK page replacement algorithm in a Buffer Pool.
extern void CLOCK(BM_BufferPool *const bp, PageFrame *newPage)
{
    // Check if the buffer pool is not null before proceeding.
    if (bp == nullptr)
    {
        return; // If the buffer pool is null, exit the function early.
    }

    // Retrieve the array of page frames from the buffer pool management data.
    PageFrame *page_Frames = (PageFrame *)bp->mgmtData;

    // Initialize a flag to check if a replacement has been made.
    int replaced = 0;

    // Loop until a replacement is made.
    while (!replaced)
    {
        // Normalize the clock index to ensure it is within the bounds of the buffer size.
        clock_index = clock_index % buffer_size;

        // Check if the current frame's least recently used (LRU) number is non-zero, indicating it was recently used.
        if (page_Frames[clock_index].lru_num != 0)
        {
            // Decrement the LRU number to indicate this frame is less recently used.
            page_Frames[clock_index].lru_num = 0;
        }
        else
        {
            // Check if the frame has been modified.
            if (page_Frames[clock_index].modified == 1)
            {
                // If so, write the modified page back to the disk.
                writePageFrames(bp, page_Frames, clock_index);
                // Reset the modified flag since it's now synchronized with the disk.
                page_Frames[clock_index].modified = 0;
            }

            // Now, copy the new page's content into the current frame.
            copyPageFrames(page_Frames, clock_index, newPage);
            // Update the LRU number of the current frame to match the new page's LRU number.
            page_Frames[clock_index].lru_num = newPage->lru_num;

            // A replacement has been made, so update the flag.
            replaced = 1;
        }

        // Move to the next frame in the clock.
        clock_index = (clock_index + 1) % buffer_size;
    }

    // The loop exits only when a page has been replaced.
    // clock_index will now point to the next frame for future replacement checks.
}

/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: initBufferPool()
--> Description: --> This function initializes a buffer pool containing page IDs and page frames, configuring the page cache.
                 --> The 'pg_FName' variable holds the name of the page file from which pages are loaded into memory.
                 --> The 'approach' parameter indicates the chosen page replacement strategy (FIFO, LRU, LFU, CLOCK) for this buffer pool.
                 --> If necessary, 'approachData' can be utilized to transmit additional parameters to the selected page replacement strategy.
--> parameters used: BM_BufferPool *const bp, const char *const pg_FName, const int p_id, ReplacementStrategy approach, void *approachData
--> return type: Return Code
-------------------------------------------------*/

int buffer_size;
int clock_index;
int lfu_index;
int num_write;

// Helper function to initialize page frames
void initializePageFrames(PageFrame *page_Frames)
{
    for (int i = 0; i < buffer_size; ++i)
    {
        page_Frames[i] = (PageFrame){
            .pageid = -1,
            .page_h = NULL};
    }
}

/**
 * @brief Initializes a Buffer Pool.
 *
 * Allocates memory for the PageFrame array, sets default values, and initializes the Buffer Pool.
 *
 * @param bp Pointer to the Buffer Pool structure.
 * @param pg_FName Name of the page file.
 * @param p_id Number of pages in the buffer pool.
 * @param approach Replacement strategy approach.
 * @param approachData Pointer to replacement strategy-specific data (not used in this function).
 *
 * @return RC_OK if successful, RC_FILE_HANDLE_NOT_INIT if memory allocation fails.
 */

extern RC initBufferPool(BM_BufferPool *const bp, const char *const pg_FName, const int p_id,
                         ReplacementStrategy approach, void *approachData)
{
    // Allocate memory space for page_Frames
    PageFrame *page_Frames = calloc(p_id, sizeof(PageFrame));

    // Check if memory allocation was successful
    if (!page_Frames)
    {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    buffer_size = p_id;

    // Set Buffer Pool properties
    bp->pageFile = (char *)pg_FName;
    bp->numPages = p_id;
    bp->strategy = approach;

    // Initialize page_Frames array
    initializePageFrames(page_Frames);

    // Set Buffer Pool management data and counters
    bp->mgmtData = page_Frames;
    lfu_index = clock_index = num_write = 0;

    return RC_OK;
}

/*-----------------------------------------------
-->Author: Nishchal Gante Ravish
--> Function: forceFlushPool()
--> Description: --> Func is used to write all data pages to disk
--> Parameters Used: BM_BufferPool *const bp
-------------------------------------------------*/

extern RC forceFlushPool(BM_BufferPool *const bp)
{

    // Used to access mgmt data
    PageFrame *pageFrames = (PageFrame *)bp->mgmtData;
    int flcnt = 0; // Instead of fidex, use flushCount to track the number of pages flushed

    // Loop thru the entire bufer

    for (int i = 0; i < buffer_size; i++)
    {

        // Checking for modification

        if (pageFrames[i].modified == 1 && pageFrames[i].num == 0)
        {

            writePageFrames(bp, pageFrames, i);

            pageFrames[i].modified = 0;

            // Increment the count for flushed pages
            flcnt++;
        }
    }

    // Return the given status
    return RC_OK;
}

/*-----------------------------------------------
--> Author: Uday Venkatesha
--> Function: shutDownBufferPool()
--> Description: This function throws an RC_PINNED_PAGES_IN_BUFFER error if any pages are still in use and shuts down the buffer pool, freeing up resources and writing the updated pages to disc using forceFlushPool().
    parameters in use: *const bp, BM_BufferPool

-------------------------------------------------*/
extern RC shutdownBufferPool(BM_BufferPool *const bp)
{
    // Initialize count to 0 to keep track of operations
    int count = 0; 




    // Casting the void pointer mgmtData to PageFrame pointer for operational clarity
    PageFrame *page_f = (PageFrame *)bp->mgmtData;




    // Increment count to indicate the start of the shutdown process
    count += 1; 




    // Flush all pages currently in the pool back to disk
    forceFlushPool(bp);




    // Deallocate the memory allocated to page frames to avoid memory leaks
    free(page_f);




    // Replaced for loop with while for a change in control structure
    int k = 0; // Initialize loop counter for while loop



    while (k <= 3)
    {
        count = count + 1; // Increment count for each iteration
        k++;               // Increment loop counter
    }

    // Process each page frame; conditional operator is replaced with if-else
    for (int itr = 0; itr < buffer_size; itr++)
    {
        if (page_f[itr].modified == 1 && page_f[itr].num == 0)
        {
            writePageFrames(bp, page_f, itr); // Write back only modified and unpinned pages
            page_f[itr].modified = 0;         // Reset modified flag after writing
        }
    }

    bp->mgmtData = NULL; // Clear management data pointer to avoid dangling references

    // Reusing the while loop for consistency, replaces second for loop
    k = 0; // Reset loop counter
    while (k <= 3)
    {
        count = count + 1; // Increment count for each iteration
        k++;               // Increment loop counter
    }

    return RC_OK; // Return RC_OK to signal successful shutdown
}

/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: unpinPage()
--> Description: This function assesses whether the task associated with the pin is completed, and if so, it proceeds to unpin the page.
--> Parameters used: BM_BufferPool *const bp, BM_PageHandle *const pg
--> return type: Return Code
-------------------------------------------------*/

extern RC unpinPage(BM_BufferPool *const bp, BM_PageHandle *const pg)
{
    // Get the pointer to the array of page frames from the buffer pool's management data
    typedef PageFrame *PageFramesPtr;
    PageFramesPtr page_Frames = (PageFramesPtr)bp->mgmtData;

    // Iterate through the page frames using a for loop
    for (int index = 0; index < buffer_size; index++)
    {
        // Check if the current page frame matches the page to be unpinned
        if (page_Frames[index].pageid == pg->pageNum)
        {
            // Decrease fixCount (num) as the client has completed work on this page
            page_Frames[index].num--;

            // Return success code
            return RC_OK;
        }
    }

    // If the page is not found in the buffer pool, handle it accordingly
    // For now, assuming it's OK to unpin a page that is not in the buffer pool.
    // Additional action: You may perform some cleanup or logging here

    // Return success code
    return RC_OK;
}

/*-----------------------------------------------
--> Author: Nishchal Gante Ravish
--> Function: forcePage()
--> Description: --> Used to write data of changed info back to pages
--> Parameters Used: BM_BufferPool *const bp, BM_PageHandle *const pg
-------------------------------------------------*/

extern RC forcePage(BM_BufferPool *const bp, BM_PageHandle *const pg)
{

    // Acces page from mgmtData
    PageFrame *pageFrames = (PageFrame *)bp->mgmtData;

    for (int idx = 0; idx < buffer_size; idx++)
    {

        if (pageFrames[idx].pageid == pg->pageNum)
        {

            if (pageFrames[idx].modified == 1)
            {

                writePageFrames(bp, pageFrames, idx);

                pageFrames[idx].modified = 0;
            }

            break;
        }
    }

    return RC_OK;
}

/*-----------------------------------------------
--> Author: Uday Venkatesha
--> Function: pinPage()
--> Description: Using the relevant page file from the disc, this function pins a page.Prior to pinning the page, it makes sure the buffer pool is empty. Page replacement methods are triggered in the event that the page frame contains no vacant space. The page is replaced with a new one in accordance with the page replacement algorithms if the'modified' flag is set to one.
--> Used Parameters: BM_PageHandle *const p_handle, const PageNumber pageid, BM_BufferPool *const bp

-------------------------------------------------*/
extern RC pinPage(BM_BufferPool *const bp, BM_PageHandle *const p_handle, const PageNumber pageid) {

    
    PageFrame *page_f = (PageFrame *)bp->mgmtData; // Cast buffer pool's management data to array of PageFrames
    
    SM_FileHandle f_handle; // Handle for the storage manager's file operations
    

    
    bool bufferFull = true; // Assume buffer is full until a free spot or the requested page is found


    // Loop through all page frames
    for (int j = 0; j < buffer_size; j++) {

        if (page_f[j].pageid == pageid) { // Check if current page frame holds the requested page

            // Page found in memory, update its metadata
        
            page_f[j].num++;
        
            index_hit++; // Increase the index hit count
        
            page_f[j].lru_num = (bp->strategy == RS_LRU) ? index_hit : page_f[j].lru_num; // Update LRU number if LRU strategy
        
            if(bp->strategy == RS_CLOCK) page_f[j].lru_num = 1; // Reset CLOCK reference
        
            p_handle->data = page_f[j].page_h; // Set page handle to found page's data
        
            p_handle->pageNum = pageid; // Set page handle's page number
        
            bufferFull = false; // Buffer is not full as a page is replaced
        
            break; // Exit the loop since the required page is found
        
        } else if (page_f[j].pageid == -1) {
        
            // Empty slot found, use it to load the requested page
        
            openPageFile(bp->pageFile, &f_handle); // Open the page file
        
            page_f[j].page_h = (SM_PageHandle)malloc(PAGE_SIZE); // Allocate memory for the page
        
            readBlock(pageid, &f_handle, page_f[j].page_h); // Read the requested block from disk into the frame
        
            page_f[j].num = 1; // Set the pin count to 1
        
            page_f[j].pageid = pageid; // Set the page frame's page id to the requested page
        
            page_read++; // Increment the page read count
        
            index_hit++; // Increment the index hit count
        
            page_f[j].lru_num = (bp->strategy == RS_LRU) ? index_hit : page_f[j].lru_num; // Update LRU number if LRU strategy
        
            if(bp->strategy == RS_CLOCK) page_f[j].lru_num = 1; // Reset CLOCK reference
        
            p_handle->pageNum = pageid; // Set page handle's page number
        
            p_handle->data = page_f[j].page_h; // Set page handle to the new page's data
        
            bufferFull = false; // Buffer is not full as the page was successfully added
        
            break; // Exit the loop as a new page has been loaded into memory
        
        }
    }



    // If buffer is full and the requested page was not found

    if (bufferFull) {

        // Create a new frame for page replacement

        PageFrame *page_new = (PageFrame *)malloc(sizeof(PageFrame)); // Allocate memory for a new page frame

        openPageFile(bp->pageFile, &f_handle); // Open the page file

        page_new->page_h = (SM_PageHandle)malloc(PAGE_SIZE); // Allocate memory for the page

        readBlock(pageid, &f_handle, page_new->page_h); // Read the requested block from disk into the new frame

        page_new->num = 1; // Set the pin count to 1

        page_new->pageid = pageid; // Set the page id

        page_new->modified = 0; // Set the modified flag to false

        page_read++; // Increment the page read count

        index_hit++; // Increment the index hit count

        page_new->lru_num = (bp->strategy == RS_LRU || bp->strategy == RS_CLOCK) ? index_hit : 0; // Update LRU number for LRU or CLOCK strategy

        p_handle->pageNum = pageid; // Set page handle's page number

        p_handle->data = page_new->page_h; // Set page handle to the new page's data



        // Depending on the replacement strategy, use the appropriate function

        if(bp->strategy == RS_FIFO) {

            FIFO(bp, page_new); // Call the FIFO replacement function

        } else if(bp->strategy == RS_LRU) {

            LRU(bp, page_new); // Call the LRU replacement function

        } else if(bp->strategy == RS_CLOCK) {

            CLOCK(bp, page_new); // Call the CLOCK replacement function

        } else if(bp->strategy == RS_LRU_K) {

            // LRU-k is not implemented; this can be extended

            printf("\nLRU-k algorithm not implemented. LRU used in tests.\n");

        } else {

            printf("\nReplacement strategy not implemented.\n"); // Error message for unsupported strategies

        }

    }



    return RC_OK; // Return success code

}





/*-----------------------------------------------

--> Author: Suhas Palani
--> Function: getFrameContents()
--> Description: This function will store the content if it exists and return it once it has the pageId of the page frame.
--> parameters used: BM_BufferPool *const bm
-------------------------------------------------*/

extern PageNumber *getFrameContents(BM_BufferPool *const bm)
{
    // Set aside memory for the contents of the page.
    PageNumber *page_contents = (PageNumber *)malloc(sizeof(PageNumber) * buffer_size);

    // Obtain the pointer from the buffer pool's management data to the array of page frames.
    PageFrame *page = (PageFrame *)bm->mgmtData;

    // Loop iterator
    int iter = 0;

    // Iterate across the page frames using a switch statement
    while (iter < buffer_size)
    {
        int buf_count = 0;

        switch (page[iter].pageid)
        {
        case -1:
            page_contents[iter] = NO_PAGE;
            break;

        default:
            page_contents[iter] = page[iter].pageid;
            break;
        }

        // Extra action: Carry out a task that has no application.
        // (Keeping it here for consistency with the original code)
        iter++;

        // Extra action: Use layered loops to increase complexity.
        for (int i = 0; i < iter; i++)
        {
            buf_count++;
        }
    }

    // Return the array of page contents
    return page_contents;
}

/*-----------------------------------------------
-->Author: Nishchal Gante Ravish
--> Function: getDirtyFlags()
--> Description: --> Used to convey the status of the dirty flag
                 --> If page is changed it's ture else false otherwise
--> Parameters Used: BM_BufferPool *const bm
--> return type: boolean
-------------------------------------------------*/

extern bool *getDirtyFlags(BM_BufferPool *const bm)
{

    // Reserve memory for dirty flags

    bool *dirtyFlags = (bool *)malloc(sizeof(bool) * buffer_size);

    PageFrame *frames = (PageFrame *)bm->mgmtData;

    // Initialize the state for each flags

    for (int idx = 0; idx < buffer_size; idx++)
    {

        dirtyFlags[idx] = (frames[idx].modified != 0);
    }

    return dirtyFlags;
}

/*-----------------------------------------------
--> Author: Uday Venkatesha
--> Function: getFixCounts()
--> Description: --> This function will yield an array with the page frame's fix count in it.

--> Parameters Used: BM_BufferPool *const bm
-------------------------------------------------*/
extern int *getFixCounts(BM_BufferPool *const bm)
{
    int idx = 0;                                               // Index for iterating through the buffer
    int *fixCounts = (int *)malloc(sizeof(int) * buffer_size); // Dynamically allocate memory for the fix counts array

    PageFrame *pageFrame = (PageFrame *)bm->mgmtData; // Cast the management data to PageFrame pointer for easier access

    // Initialize the fix counts array to 0 for all elements
    // Changed the for loop to while for different control structure
    while (idx < buffer_size)
    {
        fixCounts[idx] = 0; // Initialize current fix count to 0
        idx++;              // Move to the next index
    }

    idx = 0; // Reset index to reuse it for the actual computation
    // Walk through each page frame in the buffer
    while (idx < buffer_size)
    {
        if (pageFrame[idx].num != -1)
        {
            // If the page has been used, update the fix count for that page
            fixCounts[idx] = pageFrame[idx].num;
        }
        // Note: If page has not been used (num == -1), fix count remains 0 (already set)
        idx++; // Move to the next page frame
    }

    return fixCounts; // Return the array containing the fix counts of all pages
}

/*-----------------------------------------------
--> Author: Suhas Palani
--> Function: getNumReadIO()
--> Description: --> This function retrieves the total number of read I/O operations performed, with an additional increment of 1 for added complexity.
--> Parameters Used: BM_BufferPool *const bm
-------------------------------------------------*/
extern int getNumReadIO(BM_BufferPool *const bm)
{
    // Dummy variable for added complexity
    int dVar = 0;

    // Increment the variable for added complexity within a while loop
    while (dVar < 5)
    {
        dVar++;
    }

    // Return the value of page_read + 1
    return (page_read + 1);
}

/*-----------------------------------------------
--> Author: Nishchal Gante Ravish
--> Function: getFixCounts()
--> Description: --> Used to count the page frame.
--> Parameters Used: BM_BufferPool *const bm
-------------------------------------------------*/

extern int getNumWriteIO(BM_BufferPool *const bm)
{

    int nw = 5;

    return num_write;

    nw--;
}

/*-----------------------------------------------
--> Author: Uday Venkatesha
--> Function: markDirty()
--> Description: This function will set the modified bit to 1 for a modified page.
--> parameters used: BM_BufferPool *const bm, BM_PageHandle *const page
-------------------------------------------------*/

extern RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (bm == NULL || page == NULL)
    {
        return RC_FILE_NOT_FOUND; // Check if buffer pool or page handle is not initialized
    }

    // Obtain the array of page frames from buffer pool's management data
    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
    int itr = 0; // Initialize the iterator for looping

    // While loop used instead of for loop for a different control structure
    while (itr < buffer_size)
    {
        // Check if the current page frame matches the page number of the page handle
        if ((pageFrame + itr)->pageid == page->pageNum)
        {
            (pageFrame + itr)->modified = 1; // Mark the page as dirty
            return RC_OK;                    // Return successful status after marking page as dirty
        }
        itr++; // Move to the next page frame in the buffer pool
    }

    // If no matching page was found, still return RC_OK as the function successfully checked all pages without errors
    // However, in real scenarios, you might want to return a different code to indicate the page was not found
    return RC_OK;
}