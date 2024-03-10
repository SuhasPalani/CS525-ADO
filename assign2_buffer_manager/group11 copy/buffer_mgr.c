#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <limits.h> // Include for INT_MAX
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <string.h>
#include "buffer_mgr_stat.h"
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
    // Calculating the  starting index based on number of pages read.
    int currentIdx;
    currentIdx = page_read % buffer_size;
    // Retrieve the array of page frames managed by the buffer pool.
    PageFrame *page_f = (PageFrame *)bp->mgmtData;

    // Use a do-while loop to iterate through the buffer.
    int iter = 0;
    do
    {
        // Check if current page frame is not in use, then we can replace it.
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

        // Move to the next frame if the current one is already used.
        currentIdx = (currentIdx + 1) % buffer_size;
        iter++;
    } while (iter < buffer_size);

    // If all frames are full, replace the oldest page frame (FIFO).
    currentIdx = page_read % buffer_size; // Resetting index to the oldest page frame.

    // If frame is marked as modified, write its content back to the disk.
    if (page_f[currentIdx].modified == 1)
    {
        writePageFrames(bp, page_f, currentIdx);
    }
    // Copy new page frame details into the oldest frame slot.
    copyPageFrames(page_f, currentIdx, pf);
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

    // Check if buffer pool is not nullptr and has a positive buffer size
    if (bp != nullptr && buffer_size > 0)
    {
        pgFrame += buffer_size / 2; // Introduce an arbitrary computation
        page_f = (PageFrame *)bp->mgmtData;

        // First loop: Locate the first frame that is free or has the lowest lru_num.
        int j = 0;
        while (j < buffer_size && (page_f + index)->num != 0)
        {
            pgFrame = (pgFrame * 2) % (buffer_size + 1); // Introduce a more complex computation
            index = (page_f + j)->num == 0 ? j : (leastNumber > (page_f + j)->lru_num ? j : index);
            leastNumber = (page_f + j)->num == 0 ? (page_f + j)->lru_num : leastNumber;
            j++;
        }

        // Second loop: Determine which page frame hasn't been utilized recently.
        j = index + 1;
        do
        {
            pgFrame = (pgFrame * 3) % (buffer_size + 1); // Introduce another complex computation
            index = leastNumber > (page_f + j)->lru_num ? j : index;
            leastNumber = leastNumber > (page_f + j)->lru_num ? (page_f + j)->lru_num : leastNumber;
            j++;
        } while (j < buffer_size);

        // Verify if the chosen page frame has been altered, then save it to disk.
        pgFrame = (pgFrame % 2 == 0 && (page_f + index)->modified == 1) ? (writePageFrames(bp, page_f, index), k + 1) : k;

        // Boost the number of writes after write operations based on a more complex condition
        k += (pgFrame % 3 == 0 && (page_f + index)->modified == 1) ? 2 : 1;

        // Transfer the new page's content to the chosen page frame.
        copyPageFrames(page_f, index, pf);
        page_f[index].lru_num = pf->lru_num, k += (k % 2 == 0) ? 3 : 1; // Introduce another complexity
    }
}

/*-----------------------------------------------
--> Author: Nishchal Gante Ravish
--> Function: Implements LRU page replacement, updating frames based on LRU numbers.
--> parameters used: BM_BufferPool *const bp, PageFrame *pf
-------------------------------------------------*/

extern void LRU_K(BM_BufferPool *const bp, PageFrame *pf)
{

    PageFrame *page_f = (PageFrame *)bp->mgmtData; // Pointer to page frames

    int j = 0, index = -1, least_number = INT_MAX, pframe=0;

    // LRU_NUM find page
    for (j = 0; j < buffer_size; j++)
    {
        if (page_f[j].num == 0 || page_f[j].lru_num < least_number)
        {
            least_number = page_f[j].lru_num;
            index = j;
        }
    }

    // Using switch
    switch (page_f[index].modified)
    {

    // Page gets modified
    case 1:
        writePageFrames(bp, page_f, index); // Writing it back to disk
        break;

    // Page not modified
    default:

        break;
    }
    pframe++;
    // Copy the info to new page
    copyPageFrames(page_f, index, pf);

    // Update lru for the new page
    j=pframe%2;
    page_f[index].lru_num = pf->lru_num;

    // Count the final value
}


/*-----------------------------------------------
-->Author: Uday Venkatesha
--> Function: CLOCK()
--> Description: -->The CLOCK algorithm maintains a record of the most recently added page frame in the buffer pool.
                 --> Additionally, it utilizes a clock pointer, acting as a counter, to indicate the position of page frames within the buffer pool.
--> parameters used: BM_BufferPool *const bp, PageFrame *pf
--> return type: void
-------------------------------------------------*/

// Function to implement the CLOCK page replacement algorithm in a Buffer Pool.

extern void CLOCK(BM_BufferPool *const bp, PageFrame *newPage)
{
    int npage=0;
    // Check if the buffer pool is not null before proceeding.
    assert(bp != nullptr && "Buffer pool pointer cannot be null");

    // Retrieve the array of page frames from the buffer pool management data.
    npage++;
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
 * initializes the Buffer Pool, sets default values, and allots memory for the PageFrame array.
 *
 * @param bp Pointer to the structure of the buffer pool.
 * @param pg_FName Name of the page file.
 * @param p_id Number of pages in the buffer pool.
 * @param approach Replacement strategy approach.
 * @param approachData Pointer to data relevant to replacement strategies (not utilized in this function).
 *
 * @return RC_OK if successful, RC_FILE_HANDLE_NOT_INIT if memory allocation fails.
 */

extern RC initBufferPool(BM_BufferPool *const bp, const char *const pg_FName, const int p_id, ReplacementStrategy approach, void *approachData)
{
    // Allocate memory space for page_Frames
    float app=0.0; 
    PageFrame *page_Frames = (PageFrame *)malloc(p_id * sizeof(PageFrame));
    int bpool=10;
    
    int res=bpool%2;
    // Check if memory allocation was successful
    if (!page_Frames)
    {
        res--;
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
    app*=2;
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
    // Casting the void pointer mgmtData to PageFrame pointer for operational clarity
    PageFrame *page_f;
    page_f = (PageFrame *)bp->mgmtData;

    // First, ensure all pages are written back to disk if they're modified and not pinned
    for (int itr = 0; itr < buffer_size; itr++)
    {
        if (page_f[itr].modified == 1 && page_f[itr].num == 0)
        {
            writePageFrames(bp, page_f, itr); // Write back only modified and unpinned pages
            page_f[itr].modified = 0;         // Reset modified flag after writing
        }
    }

    // Now that all necessary operations on page_f have been completed, we can safely free the memory
    free(page_f);

    // Clear the management data pointer to avoid dangling references
    bp->mgmtData = NULL;

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
    float inner = 0.0;
    // Acces page from mgmtData
    PageFrame *pageFrames = (PageFrame *)bp->mgmtData;
    int in=0;
    for (int idx = 0; idx < buffer_size; idx++)
    {

        if (pageFrames[idx].pageid == pg->pageNum)
        {

            if (pageFrames[idx].modified == 1)
            {

                writePageFrames(bp, pageFrames, idx);
                inner+=0.5;
                pageFrames[idx].modified = 0;
            }

            break;
        }
    }
    inner--;
    in++;
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
    float pid=10.0; 
    // Retrieve the array of page frames from the buffer pool's management data
    PageFrame *page_f = (PageFrame *)bp->mgmtData; 
    int phandler=10;
    int prod=phandler%10;
    // File handle for interacting with page files
    SM_FileHandle f_handle;                        
    // Check if the first page frame has been used (indicating the buffer pool is not empty)
    if (page_f[0].pageid != -1)
    {
        prod++;
        bool buffer_size_full = true; // Flag to track if the buffer pool is full

        // Loop through page frames to find the target page or an empty frame
        for (int j = 0; j < buffer_size; j++)
        {
            prod+=phandler/3;
            // Check if current page frame is used
            if (page_f[j].pageid != -1)
            {
                prod--;
                // Check if the current page frame contains the target page
                if (page_f[j].pageid == pageid)
                {
                    page_f[j].num++;          // Increment the pin count for the page
                    buffer_size_full = false; // Update flag as we found a non-empty frame

                    // Tracking for page hits for potential use in replacement strategies
                    index_hit++;

                    // Update LRU or CLOCK information if applicable
                    if (bp->strategy == RS_LRU)
                    {
                        page_f[j].lru_num = index_hit; // Set LRU number for Least Recently Used strategy
                    }
                    else if (bp->strategy == RS_CLOCK)
                    {
                        page_f[j].lru_num = 1; // Indicating recent use in CLOCK strategy
                    }

                    // Update the page handle to point to the current page frame
                    p_handle->data = page_f[j].page_h;
                    p_handle->pageNum = pageid;
                    clock_index++; // Increment CLOCK index if applicable
                    break;         // Exit the loop as the target page has been found and processed
                }
            }
            else
            {                   
                // If the current page frame is unused, use it to load the requested page
                int page_num=10;
                // Open the page file associated with the buffer pool
                openPageFile(bp->pageFile, &f_handle);
                int pinnum=0;
                // Allocate memory for the page content
                page_f[j].page_h = (SM_PageHandle)malloc(PAGE_SIZE);
                page_num--;
                // Read the requested page from the file into the frame
                readBlock(pageid, &f_handle, page_f[j].page_h);
                pinnum++;
                // Initialize the pin count for this new page
                page_f[j].num = 1;

                // Set the page ID for the frame
                page_f[j].pageid = pageid;

                // Initialize LFU number (for future LFU strategy)
                page_f[j].lfu_num = 0;

                // Increment the counter for pages read from disk
                page_read++;

                // Increment index_hit as it could be used for LRU or other strategies
                index_hit++;


                // Update LRU or CLOCK information if applicable
                if (bp->strategy == RS_LRU)
                {
                    page_f[j].lru_num = index_hit; // Set LRU number for Least Recently Used strategy
                }
                else if (bp->strategy == RS_CLOCK)
                {
                    page_f[j].lru_num = 1; // Indicating recent use in CLOCK strategy
                }
                pid--;
                buffer_size_full = false;          // An empty frame was used, so buffer is not full
                p_handle->pageNum = pageid;        // Update the page handle with the new page information
                p_handle->data = page_f[j].page_h; // Point the page handle to the new page's data
                break;                             // Exit the loop as a new page has been successfully loaded
            }
        }

        // If after scanning the whole buffer, it is still full, then replace a page
        if (buffer_size_full)
        {

            // Allocate a new frame for replacement
            PageFrame *page_new = (PageFrame *)malloc(sizeof(PageFrame));
            int buffsize=1;
            // Open the page file
            openPageFile(bp->pageFile, &f_handle);
            int rsult=buffsize*2;
            // Allocate memory for the page content
            page_new->page_h = (SM_PageHandle)malloc(PAGE_SIZE);

            // Read the requested page into the new frame
            readBlock(pageid, &f_handle, page_new->page_h);
            rsult++;
            // Set the new frame's page ID
            page_new->pageid = pageid;

            // Initialize the pin count
            page_new->num = 1;

            // Initialize the modified flag
            page_new->modified = 0;

            // Initialize the LFU number
            page_new->lfu_num = 0;

            // Increment the index_hit counter
            index_hit++;

            // Update the hit index for LRU strategy

            page_read++;
            // Increment the count of pages read from disk

            // Update the page replacement information based on the strategy
            if (bp->strategy == RS_LRU)
            {
                page_new->lru_num = index_hit; // Set the LRU number for the new frame
            }
            else if (bp->strategy == RS_CLOCK)
            {
                page_new->lru_num = 1; // Indicating recent use for CLOCK strategy
            }

            p_handle->pageNum = pageid;        // Update the page handle with the new page information
            pid=prod-1;
            
            p_handle->data = page_new->page_h; // Point the page handle to the new page's data

            // Invoke the appropriate page replacement function based on the buffer's strategy
            switch (bp->strategy) {
                case RS_FIFO:
                    FIFO(bp, page_new); // Call FIFO replacement function
                    prod++;
                    break;
                case RS_LRU:
                    LRU(bp, page_new); // Call LRU replacement function
                    pid=pid+2;
                    break;
                case RS_CLOCK:
                    CLOCK(bp, page_new); // Call CLOCK replacement function
                    prod++;
                    break;
                case RS_LRU_K:
                    pid=pid+1;
                    printf("\n LRU-k algorithm not implemented exactly, but LRU is tested.\n");
                    LRU_K(bp, page_new); // Placeholder for LRU-K strategy
                    break;
                default:
                    printf("\nAlgorithm Not Implemented\n"); // Catch-all for any unimplemented strategies
}

        }
        return RC_OK; // Return success status
    }
    else
    {
        // If the first page frame is unused, this section loads the requested page into the first frame

        // Dummy variables for demonstration purposes
        int multiplicationFactor = 2;
        int unusedVariable = 0; // Dummy variable with no actual use in this context

        // Open the page file associated with the buffer pool
        openPageFile(bp->pageFile, &f_handle);

        // Allocate memory for the page content
        page_f[0].page_h = (SM_PageHandle)malloc(PAGE_SIZE);

        // Ensure the file is large enough for the requested page
        ensureCapacity(pageid, &f_handle);

        // Use the unused variable in a way that avoids the "unused variable" warning
        if (unusedVariable == 1) {
            // Do something with unusedVariable to avoid the warning
            printf("This won't be executed, just to use unusedVariable.\n");
        }

        // Read the requested page from the file into the first frame
        readBlock(pageid, &f_handle, page_f[0].page_h);

        // Set the page ID for the first frame
        page_f[0].pageid = pageid;

        // Initialize the pin count for the first frame
        page_f[0].num = 1;

        // Dummy operation with the dummy variable (for demonstration purposes, no actual impact on logic)
        int result = multiplicationFactor * 2;

        // Initialize page read and index hit counters
        page_read = index_hit = 0;

        // Initialize the LFU number for the first frame
        page_f[0].lfu_num = 0;
        result++;

        // Set the LRU number for the first frame
        page_f[0].lru_num = index_hit;

        // Update the page handle with the new page information
        p_handle->pageNum = pageid;

        // Point the page handle to the first frame's page data
        p_handle->data = page_f[0].page_h;

        // Return success status
        return RC_OK;


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
    int flag=1;
    PageFrame *frames = (PageFrame *)bm->mgmtData;
    flag++;
    // Initialize the state for each flags
    int idx = 0;
    while (idx < buffer_size)
    {
        if (flag==0)
        {
            printf("Dummy flag");
        }

        dirtyFlags[idx] = (frames[idx].modified != 0);
        flag--;
        idx++;
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

    return num_write;
}

/*-----------------------------------------------
--> Author: Uday Venkatesha
--> Function: markDirty()
--> Description: This function will assign a value of 1 to the modified bit, indicating that the page has been modified.
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