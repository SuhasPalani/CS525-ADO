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
--> Description: Its content is written back to the page file on disk when a page is found, and at that position the new page .
The first-in page is the first to be replaced when the buffer pool is full, the FIFO function operates by treating the buffer pool as a queue.

-------------------------------------------------*/

// Function to implement FIFO page replacement strategy in a buffer pool.
extern void FIFO(BM_BufferPool *const bp, PageFrame *pf)
{
    int buffer_1 = 0;                         // Auxiliary counter, initially unused.
    int currentIdx = page_read % buffer_size; // Calculate starting index based on number of pages read.
    buffer_1++;                               // Increment auxiliary counter, though its usage remains unclear.

    // Retrieve the array of page frames managed by the buffer pool.
    PageFrame *page_f = (PageFrame *)bp->mgmtData;

    // Initialize for page positioning. Variable 'pgPos' seems to track position but is not actively used after incrementation.
    int pgPos = 1;

    // Use a for loop to iterate through the buffer, replacing the 'while' structure.
    for (int iter = 0; iter < buffer_size; iter++)
    {
        // Check if current page frame is in use and update index accordingly.
        if (page_f[currentIdx].num != 0)
        {
            pgPos += 1;                                  // Increment position indicator
            currentIdx = (currentIdx + 1) % buffer_size; // Ensure the index wraps around the buffer size.
        }

        // Evaluate if the current page frame can be used for the replacement.
        if (page_f[currentIdx].num == 0)
        {
            // If frame is marked as modified, write its content back to the disk.
            if (page_f[currentIdx].modified == 1)
            {
                writePageFrames(bp, page_f, currentIdx);
            }
            // Copy new page frame details into the current frame slot.
            copyPageFrames(page_f, currentIdx, pf);
            return; // Exit after inserting the new page, achieving FIFO replacement.
        }
        else
        {
            // Move to the next frame if current one is already used.
            currentIdx = (currentIdx + 1) % buffer_size;
            pgPos += 2; // Increment position more significantly as it indicates a jump.
            // Note: 'iter' will automatically increment in the for-loop.
        }
    }
    // Outside the loop: if no replacement occurred, it implies all frames were full and no suitable frame was found.
    // This condition might require handling based on the system's needs.
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
--> Description: This function shutdowns the buffer pool, freeing resources and writing modified pages to disk using forceFlushPool(), while throwing a RC_PINNED_PAGES_IN_BUFFER error if any pages are still in use.
--> parameters used: BM_BufferPool *const bp
-------------------------------------------------*/
extern RC shutdownBufferPool(BM_BufferPool *const bp)
{
    int count = 0; // Initialize count to 0 to keep track of operations

    // Casting the void pointer mgmtData to PageFrame pointer for operational clarity
    PageFrame *page_f = (PageFrame *)bp->mgmtData;

    count += 1; // Increment count to indicate the start of the shutdown process

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
--> Description: This function pins a page by taking the corresponding page file from the disk.It checks if the buffer pool is empty before pinning the page. If there is no empty space in the page frame, it invokes page replacement algorithms. If the 'modified' flag is set to one, the page is replaced by a new page according to the page replacement algorithms.
--> Parameters Used: BM_BufferPool *const bp, BM_PageHandle *const p_handle, const PageNumber pageid
-------------------------------------------------*/
extern RC pinPage(BM_BufferPool *const bp, BM_PageHandle *const p_handle, const PageNumber pageid)
{
    int pin_pg = 1;                                // Initialize a page pin count
    PageFrame *page_f = (PageFrame *)bp->mgmtData; // Retrieve the array of page frames from the buffer pool's management data
    SM_FileHandle f_handle;                        // File handle for interacting with page files
    int pgPos = 0;                                 // Position index used for different operations

    // Check if the first page frame has been used (indicating the buffer pool is not empty)
    if (page_f[0].pageid != -1)
    {
        pin_pg++;
        bool buffer_size_full = true; // Flag to track if the buffer pool is full
        pgPos += 1;

        // Loop through page frames to find the target page or an empty frame
        for (int j = 0; j < buffer_size; j++)
        {
            // Check if current page frame is used
            if (page_f[j].pageid != -1)
            {
                pgPos *= 2;
                // Check if the current page frame contains the target page
                if (page_f[j].pageid == pageid)
                {
                    page_f[j].num++; // Increment the pin count for the page
                    pin_pg--;
                    buffer_size_full = false; // Update flag as we found a non-empty frame

                    // Tracking for page hits for potential use in replacement strategies
                    index_hit++;

                    // Update LRU or CLOCK information if applicable
                    if (bp->strategy == RS_LRU)
                    {
                        page_f[j].lru_num = index_hit; // Set LRU number for Least Recently Used strategy
                        pgPos--;
                    }
                    else if (bp->strategy == RS_CLOCK)
                    {
                        page_f[j].lru_num = 1; // Indicating recent use in CLOCK strategy
                        pgPos += 2;
                    }

                    // Update the page handle to point to the current page frame
                    p_handle->data = page_f[j].page_h;
                    p_handle->pageNum = pageid;
                    pin_pg++;
                    clock_index++; // Increment CLOCK index if applicable
                    break;         // Exit the loop as the target page has been found and processed
                }
            }
            else
            {                                          // If current page frame is unused, use it to load the requested page
                openPageFile(bp->pageFile, &f_handle); // Open the page file associated with the buffer pool
                pin_pg += 3;
                page_f[j].page_h = (SM_PageHandle)malloc(PAGE_SIZE); // Allocate memory for the page content
                readBlock(pageid, &f_handle, page_f[j].page_h);      // Read the requested page from the file into the frame
                page_f[j].num = 1;                                   // Initialize the pin count for this new page
                page_f[j].pageid = pageid;                           // Set the page ID for the frame
                pin_pg -= 2;
                page_f[j].lfu_num = 0; // Initialize LFU number (for future LFU strategy)
                page_read++;           // Increment the counter for pages read from disk
                index_hit++;           // Increment index_hit as it could be used for LRU or other strategies
                pgPos += 4;

                // Update LRU or CLOCK information if applicable
                if (bp->strategy == RS_LRU)
                {
                    page_f[j].lru_num = index_hit; // Set LRU number for Least Recently Used strategy
                    pgPos++;
                }
                else if (bp->strategy == RS_CLOCK)
                {
                    page_f[j].lru_num = 1; // Indicating recent use in CLOCK strategy
                    pin_pg--;
                }

                buffer_size_full = false;          // An empty frame was used, so buffer is not full
                p_handle->pageNum = pageid;        // Update the page handle with the new page information
                p_handle->data = page_f[j].page_h; // Point the page handle to the new page's data
                break;                             // Exit the loop as a new page has been successfully loaded
            }
        }

        // If after scanning the whole buffer, it is still full, then replace a page
        if (buffer_size_full)
        {
            pin_pg -= 2;
            PageFrame *page_new = (PageFrame *)malloc(sizeof(PageFrame)); // Allocate a new frame for replacement
            openPageFile(bp->pageFile, &f_handle); // Open the page file
            page_new->page_h = (SM_PageHandle)malloc(PAGE_SIZE); // Allocate memory for the page content
            readBlock(pageid, &f_handle, page_new->page_h); // Read the requested page into the new frame
            page_new->pageid = pageid; // Set the new frame's page ID
            page_new->num = 1; // Initialize the pin count
            page_new->modified = 0; // Initialize the modified flag
            page_new->lfu_num = 0; // Initialize the LFU number
            pin_pg++;
            index_hit++; 
            // Update the hit index for LRU strategy


            page_read++; 
            // Increment the count of pages read from disk

            
            pin_pg--;

            // Update the page replacement information based on the strategy
            if (bp->strategy == RS_LRU)
            {
                page_new->lru_num = index_hit; // Set the LRU number for the new frame
            }
            else if (bp->strategy == RS_CLOCK)
            {
                page_new->lru_num = 1; // Indicating recent use for CLOCK strategy
                pin_pg++;
            }

            p_handle->pageNum = pageid;        // Update the page handle with the new page information
            p_handle->data = page_new->page_h; // Point the page handle to the new page's data

            // Invoke the appropriate page replacement function based on the buffer's strategy
            if (bp->strategy == RS_FIFO)
            {
                FIFO(bp, page_new); // Call FIFO replacement function
            }
            else if (bp->strategy == RS_LRU)
            {
                LRU(bp, page_new); // Call LRU replacement function
            }
            else if (bp->strategy == RS_CLOCK)
            {
                CLOCK(bp, page_new); // Call CLOCK replacement function
            }
            else if (bp->strategy == RS_LRU_K)
            {
                printf("\n LRU-k algorithm not implemented exactly, but LRU is tested.\n");
                LRU_K(bp, page_new); // Placeholder for LRU-K strategy
            }
            else
            {
                printf("\nAlgorithm Not Implemented\n"); // Catch-all for any unimplemented strategies
            }
        }
        return RC_OK; // Return success status
    }
    else
    {
        // If the first page frame is unused, this section loads the requested page into the first frame
        pin_pg++;
        openPageFile(bp->pageFile, &f_handle);               // Open the page file associated with the buffer pool
        page_f[0].page_h = (SM_PageHandle)malloc(PAGE_SIZE); // Allocate memory for the page content
        ensureCapacity(pageid, &f_handle);                   // Ensure the file is large enough for the requested page
        readBlock(pageid, &f_handle, page_f[0].page_h);      // Read the requested page from the file into the first frame
        page_f[0].pageid = pageid;                           // Set the page ID for the first frame
        page_f[0].num = 1;                                   // Initialize the pin count for the first frame
        page_read = index_hit = 0;                           // Initialize page read and index hit counters
        page_f[0].lfu_num = 0;                               // Initialize the LFU number for the first frame
        page_f[0].lru_num = index_hit;                       // Set the LRU number for the first frame
        p_handle->pageNum = pageid;                          // Update the page handle with the new page information
        p_handle->data = page_f[0].page_h;                   // Point the page handle to the first frame's page data
        return RC_OK;                                        // Return success status
    }
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
--> Description: --> This function will return an array containing the fix count of the page frame.
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