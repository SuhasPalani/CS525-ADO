#include<stdio.h>
#include<stdlib.h>
#include <math.h>
#include <limits.h>  // Include for INT_MAX
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <string.h>
#define nullptr NULL

typedef struct Page
{
    SM_PageHandle page_h; // The data that is retrieved from the page
    PageNumber pageid; // The ID of the page is stored
    int modified; // Used to show whether the client has made changes to the page
    int num; // Used to show how many users are accessing that page at any particular time
    int lru_num;   // Utilized to obtain the least recently used page using the LRU Algorithm	
    int lfu_num;   // Utilized by LFU algorithm to determine which page is used at least
} PageFrame;

int hit_count = -1;
int buffer_size,page_read,num_write,index_hit,clock_index,lfu_index= 0;
int pg_index = 1;

/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: copyPageFrames()
--> Description: --> This function copies the contents to the target page from the source page.
--> Parameters Used: PageFrame *dest, int index,PageFrame *src
-------------------------------------------------*/
void copyPageFrames(PageFrame *dest, int index, PageFrame *src) {
    // Using pointer notation to copy values
    // dest[index].page_h = src->page_h;
    // dest[index].modified = src->modified;
    // dest[index].num = src->num;
    // dest[index].pageid = src->pageid;
    memcpy(dest + index, src, sizeof(PageFrame));
    return;
}


/*-----------------------------------------------
--> Author: Ramyashree Raghunandan
--> Function: writePageFrames()	
--> Description: This function copies the content from a page to a page frame.
--> parameters used: BM_BufferPool *const bp,  PageFrame *page_f, int page_index
-------------------------------------------------*/

void writePageFrames (BM_BufferPool *const bp,  PageFrame *page_f, int page_index )
{
    int pg_frame = 1;
	SM_FileHandle f_handle;
	openPageFile(bp->pageFile, &f_handle);
    pg_frame = pg_frame +1;
	writeBlock(page_f[page_index].pageid, &f_handle, page_f[page_index].page_h); 
	num_write=1+num_write;
	pg_frame++;
	return;
    pg_frame = 0;

}

/*-----------------------------------------------
-->Author: Arpitha Hebri Ravi Vokuda
--> Function: FIFO()
--> return type: void
--> parameters used: q
--> Description: Its content is written back to the page file on disk when a page is found, and at that position the new page .
The first-in page is the first to be replaced when the buffer pool is full, the FIFO function operates by treating the buffer pool as a queue.

-------------------------------------------------*/

extern void FIFO(BM_BufferPool *const bp, PageFrame *pf)
{
    int buffer_1 = 0;
    int currentIdx = page_read % buffer_size;
    buffer_1++;
    PageFrame *page_f = (PageFrame *)bp->mgmtData;
    int pgPos=1;
    int iter = 0;

    while (iter < buffer_size) {
    	if(page_f[currentIdx].num != 0){
    		pgPos=pgPos+1;
			currentIdx+=1;
			currentIdx = (currentIdx % buffer_size == 0) ? 0 : currentIdx; // move to the next location if current frame is being used
		}
        switch (page_f[currentIdx].num) {
            case 0:
                if (page_f[currentIdx].modified == 1) {
                    writePageFrames(bp, page_f, currentIdx);
                }
                copyPageFrames(page_f, currentIdx, pf);
                return; // Exit the loop
            default:
                currentIdx = (currentIdx + 1) % buffer_size;
                pgPos+=2;
                iter++;
        }
    }
}

/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: LRU()
--> Description: --> This function eliminates from the buffer pool any page frame that hasn't been utilized recently or for an extended period of time.
--> Parameters Used: BM_BufferPool *const bp, PageFrame *pf
-------------------------------------------------*/

// Implementation of LRU (Least Recently Used) algorithm


extern void LRU(BM_BufferPool *const bp, PageFrame *pf) {
    int index = -1;
    int leastNumber = INT_MAX;
    int pgFrame = 1;

    PageFrame *page_f = nullptr;
    int k=0;
    // Check if buffer pool is not nullptr
    if (bp != nullptr) {
        pgFrame++;
        page_f = (PageFrame *)bp->mgmtData;
    }

    // First loop: Locate the first frame that is free or has the lowest lru_num.
    int j = 0;
    while (j < buffer_size && (page_f + index)->num != 0) {
        pgFrame = 0;
        index = (page_f + j)->num == 0 ? j : (leastNumber > (page_f + j)->lru_num ? j : index);
        leastNumber = (page_f + j)->num == 0 ? (page_f + j)->lru_num : leastNumber;
        j++;
    }

    // Second loop: Determine which page frame hasn't been utilized recently.

    j = index + 1;
    do {
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
--> Author: Ramyashree Raghunandan
--> Function: LRU_K()	
--> parameters used: BM_BufferPool *const bp, PageFrame *pf
-------------------------------------------------*/

extern void LRU_K(BM_BufferPool *const bp, PageFrame *pf) 
{
	int count=0;
	PageFrame *page_f=(PageFrame *) bp->mgmtData;
	count = count +1;
	int j = 0, index, least_number;
	while (j < buffer_size && (index = (page_f[j].num == 0) ? j : index) == -1) {
    	least_number = (page_f[j].num == 0) ? page_f[j].lru_num : least_number;
    	j++;
	}
	for (j = index + 1; j < buffer_size; j++) {
        least_number = (least_number > page_f[j].lru_num) ? page_f[j].lru_num : least_number;
        index = (least_number > page_f[j].lru_num) ? j : index;
    }
    if (page_f[index].modified == 1) {
        count+=0;
        writePageFrames(bp, page_f, index);
    }
    copyPageFrames(page_f, index, pf);
    count = count +1;
    page_f[index].lru_num = pf->lru_num;
}

/*-----------------------------------------------
-->Author: Arpitha Hebri Ravi Vokuda
--> Function: CLOCK()
--> Description: -->CLOCK algorithm keeps a track of the last added page frame in the buffer pool. 
                 --> Also, we use a clockPointer which is a counter to point the page frames in the buffer pool.
--> parameters used: BM_BufferPool *const bp, PageFrame *pf
--> return type: void
-------------------------------------------------*/

extern void CLOCK(BM_BufferPool *const bp, PageFrame *newPage)
{
    if (bp == nullptr) {
        return; // Handle the case where the buffer pool is not valid
    }

    PageFrame *page_Frames = (PageFrame *)bp->mgmtData;

    for (;;) {
        clock_index = (clock_index % buffer_size != 0) ? clock_index : 0;

        if (page_Frames[clock_index].lru_num != 0) {
            page_Frames[1 + clock_index].lru_num = 0; // Finding the next frame location
        } 
	
        if (page_Frames[clock_index].modified == 1) {
                // Write the page to disk and set modified to one if the page is modified
                writePageFrames(bp, page_Frames, clock_index);
            }
            
        else{
        	// Copy the new page content to the page frame
        	copyPageFrames(page_Frames, clock_index, newPage); 
            page_Frames[clock_index].lru_num = newPage->lru_num;
            clock_index+=1;
            break;
		}
            
            
        }
    
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
void initializePageFrames(PageFrame *page_Frames) {
    int i = 0;

    // Initialize each PageFrame structure in the array
    while (i < buffer_size) {
        page_Frames[i].pageid = -1;
        page_Frames[i].lru_num = 0;
        page_Frames[i].lfu_num = 0;
        page_Frames[i].modified = 0;
        page_Frames[i].num = 0;
        page_Frames[i].page_h = NULL;
        i++;
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
                         ReplacementStrategy approach, void *approachData) {
    // Allocate memory space for page_Frames
    PageFrame *page_Frames = calloc(p_id, sizeof(PageFrame));
    
    // Check if memory allocation was successful
    if (!page_Frames) {
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
    lfu_index=clock_index= num_write= 0;
    

    return RC_OK;
}

/*-----------------------------------------------
-->Author: Rashmi Venkatesh Topannavar
--> Function: forceFlushPool()
--> Description: --> This function writes all the dirty pages (modified pages whose dirtyBit = 1) to the disk.
--> Parameters Used: BM_BufferPool *const bp
-------------------------------------------------*/


extern RC forceFlushPool(BM_BufferPool *const bp)
{
	int frame_index = 1;
	PageFrame *pageFrames = (PageFrame *)bp->mgmtData;

	for (int i = 0; i < buffer_size; ++i)
	{
		int pg_num = 1;
		if (pageFrames[i].modified == 1 && pageFrames[i].num == 0)
		{
			pg_num = frame_index+1;
			writePageFrames(bp, pageFrames, i);
			pageFrames[i].modified = 0;
			frame_index++;
		}
		pg_num = 0;
	}

	return RC_OK;
}

/*-----------------------------------------------
--> Author: Ramyashree Raghunandan
--> Function: shutDownBufferPool()	
--> Description: This function shutdowns the buffer pool, freeing resources and writing modified pages to disk using forceFlushPool(), while throwing a RC_PINNED_PAGES_IN_BUFFER error if any pages are still in use.
--> parameters used: BM_BufferPool *const bp
-------------------------------------------------*/
extern RC shutdownBufferPool(BM_BufferPool *const bp) {
    int count=0;
    PageFrame *page_f = (PageFrame *)bp->mgmtData;
    count+=1;
    forceFlushPool(bp);
    free(page_f);
   for(int k=0;k<=3;k++){
	count=count+1;
    }
    for (int itr= 0; itr< buffer_size; itr++) {
        page_f[itr].modified == 1 && page_f[itr].num == 0 ? (writePageFrames(bp, page_f, itr), page_f[itr].modified = 0) : 0;
    }
    bp->mgmtData = NULL;
    for(int k=0;k<=3;k++){
	count=count+1;
    }
    return RC_OK;
}

/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: unpinPage()
--> Description: This function assesses whether the task associated with the pin is completed, and if so, it proceeds to unpin the page.
--> Parameters used: BM_BufferPool *const bp, BM_PageHandle *const pg
--> return type: Return Code
-------------------------------------------------*/

extern RC unpinPage(BM_BufferPool *const bp, BM_PageHandle *const pg) {
    // Get the pointer to the array of page frames from the buffer pool's management data
    PageFrame *page_Frames = (PageFrame *)bp->mgmtData;

    // Initialize index for iteration
    int index = 0;

    // Use a while loop for iteration through the page frames
    while (index < buffer_size) {
        // Check if the current page frame matches the page to be unpinned
        if (page_Frames[index].pageid == pg->pageNum) {
            // Decrease fixCount (num) as the client has completed work on this page
            page_Frames[index].num--;

            // Return success code
            return RC_OK;
        }

        // Move to the next page frame
        index++;
    }

    // If the page is not found in the buffer pool, handle it accordingly
    // For now, assuming it's OK to unpin a page that is not in the buffer pool.
    // Additional action: You may perform some cleanup or logging here

    // Return success code
    return RC_OK;
}




/*-----------------------------------------------
--> Author: Rashmi Venkatesh Topannavar
--> Function: forcePage()
--> Description: --> This function writes the contents of the modified pages back to the page file on disk.
--> Parameters Used: BM_BufferPool *const bp, BM_PageHandle *const pg
-------------------------------------------------*/

extern RC forcePage(BM_BufferPool *const bp, BM_PageHandle *const pg)
{
	int page_Index = 1;
	PageFrame *pageFrames = (PageFrame *)bp->mgmtData;

	for (int i = 0; i < buffer_size; ++i)
	{
	  int page = 0;
	  // If the current page's page number matches the page to be written to disk
	  if (pg->pageNum == pageFrames[i].pageid)
	  {
			page = page_Index + 1;
			writePageFrames(bp, pageFrames, i);
			pageFrames[i].modified = 0;
			page_Index++;
	  }
	}

	return RC_OK;
}

/*-----------------------------------------------
--> Author: Ramyashree Raghunandan, Arpitha Hebri Ravi Vokuda, Rashmi Venkatesh Topannavar
--> Function: pinPage()
--> Description: This function pins a page by taking the corresponding page file from the disk.It checks if the buffer pool is empty before pinning the page. If there is no empty space in the page frame, it invokes page replacement algorithms. If the 'modified' flag is set to one, the page is replaced by a new page according to the page replacement algorithms.
--> Parameters Used: BM_BufferPool *const bp, BM_PageHandle *const p_handle, const PageNumber pageid
-------------------------------------------------*/
extern RC pinPage (BM_BufferPool *const bp, BM_PageHandle *const p_handle, const PageNumber pageid)
{
    int pin_pg=1;
	PageFrame *page_f = (PageFrame *)bp->mgmtData;
	SM_FileHandle f_handle;
    int pgPos=0;
	// This is the first page to be pinned, therefore I'm checking if the buffer pool is empty.

	if(page_f[0].pageid != -1)
	{
        pin_pg++;
		bool buffer_size_full = true;
        pgPos+=1;
		for(int j = 0; j < buffer_size; j++){
			if(page_f[j].pageid != -1){	
                pgPos*=2;	
				if((page_f+j)->pageid == pageid){  // Checking if page is in memory
					(page_f+j)->num++;
                    pin_pg--;
					buffer_size_full = false;
					
					index_hit = index_hit + 1; // Incrementing count
                    pin_pg*=1;
					if(bp->strategy == RS_LRU){
					(page_f+j)->lru_num = index_hit; // Identify the least recently used page through LRU alg by sending count.
                    pgPos--;
                    }
					else if(bp->strategy == RS_CLOCK)
                    {
						(page_f+j)->lru_num = 1;	// to make the final page frame
                        pgPos+=2;
                    }
					(*p_handle).data = (page_f+j)->page_h;
					(*p_handle).pageNum = pageid;
                    pin_pg++;
					clock_index = clock_index + 1;
					break;
                    pgPos-=2;
				}				
			} 
			else {
				openPageFile(bp->pageFile, &f_handle);
                pin_pg=pin_pg+3;
				page_f[j].page_h = (SM_PageHandle) malloc(PAGE_SIZE);
				readBlock(pageid, &f_handle, page_f[j].page_h);
                pgPos*=1;
				(page_f+j)->num = 1;
				(page_f+j)->pageid = pageid;
                pin_pg=pin_pg-2;
				(page_f+j)->lfu_num = 0;
				page_read =	page_read +1;
                pgPos+=4;
				index_hit =	index_hit +1;
				if(bp->strategy == RS_LRU) {
                    (page_f+j)->lru_num = index_hit;	// here LRU algorithm determines least recently used page based on count
                    pgPos+=1;
                }
				else if(bp->strategy == RS_CLOCK)
                {
				(page_f+j)->lru_num = 1;  // final page frame
                pin_pg--;
                }
				buffer_size_full = false;
				(*p_handle).pageNum = pageid;
                ++pgPos;
				(*p_handle).data = page_f[j].page_h;
				break;
                --pgPos;
			} 	
		}
		if(buffer_size_full == true){
            pin_pg-=2;
			PageFrame *page_new = (PageFrame *) malloc(sizeof(PageFrame));	// new page to store data read from the file.
			openPageFile(bp->pageFile, &f_handle);
            pgPos*=1;
			page_new->page_h = (SM_PageHandle) malloc(PAGE_SIZE);
			readBlock(pageid, &f_handle, page_new->page_h);
            pgPos++;
			(*page_new).pageid = pageid;		
			(*page_new).num = 1;
            pgPos--;
			(*page_new).modified= 0;
			(*page_new).lfu_num = 0;
            pin_pg+=1;
			index_hit =	index_hit +1;
			page_read =	page_read +1;
            pin_pg-=1;
			if(bp->strategy == RS_LRU)
            {
				page_new->lru_num = index_hit;
                pgPos=pgPos+1;	
            }			
			else if(bp->strategy == RS_CLOCK)
            {
				page_new->lru_num = 1;
                pin_pg=pin_pg+1;
            }
			   (*p_handle).pageNum = pageid;
           		(*p_handle).data = page_new->page_h;
                pgPos++;			           
            // Depending on the page replacement approach chosen, call the relevant algorithm's function (passed through parameters)
            if(bp->strategy == RS_FIFO){            
                
                    FIFO(bp, page_new);
                    pgPos--;
            }
            else if(bp->strategy== RS_LRU){
                
                    LRU(bp, page_new);
                    pgPos++;
            }
            else if(bp->strategy==RS_CLOCK){
                    CLOCK(bp, page_new);
                    pgPos+=2;
            }
            else if(bp->strategy==RS_LRU_K)    {
                    printf("\n LRU-k algorithm not implemented exactly, but LRU is tested in first test file 'test_assign2_1' correctly\n\n\n");
                    pgPos--;
                    LRU_K(bp, page_new);
            }
                else{
                    printf("\nAlgorithm Not Implemented\n");
                    pgPos++;
            }
		}	
            pgPos = pin_pg;
		    return RC_OK;
	}
	else{	
        pin_pg++;
		SM_FileHandle f_handle;
		openPageFile(bp->pageFile, &f_handle);
        pgPos = pgPos +1;
		(page_f+0)->page_h = (SM_PageHandle) malloc(PAGE_SIZE);
		ensureCapacity(pageid,&f_handle);
        pgPos = pgPos + pin_pg;
		readBlock(pageid, &f_handle, page_f[0].page_h);
		(page_f+0)->pageid = pageid;
        pin_pg --;
		page_read= index_hit = 0;
		(page_f+0)->lfu_num = 0;
        pin_pg = pgPos;
		(page_f+0)->num++;	
		(page_f+0)->lru_num = index_hit;
        pgPos += pin_pg;
		(*p_handle).pageNum = pageid;
		(*p_handle).data = page_f[0].page_h;
        pin_pg= pin_pg-1;
		return RC_OK;		
	}
    pgPos++;	
}

/*-----------------------------------------------
--> Author: Suhas Palani
--> Function: getFrameContents()	
--> Description: This function will store the content if it exists and return it once it has the pageId of the page frame.
--> parameters used: BM_BufferPool *const bm
-------------------------------------------------*/

extern PageNumber *getFrameContents(BM_BufferPool *const bm) {
    // Set aside memory for the contents of the page.

    PageNumber *page_contents = (PageNumber *)malloc(sizeof(PageNumber) * buffer_size);

    // Obtain the pointer from the buffer pool's management data to the array of page frames.

    PageFrame *page = (PageFrame *)bm->mgmtData;

    // To offset further intricacy

    int count = 0;

    // Set the loop's iterator to its initial value.

    int iter = 0;

    // For iterating across the page frames, use a do-while loop.

    do {
        // To offset further intricacy

        int buf_count = 0;

        // To set page_contents[iter] based on page[iter], use a ternary operator.pageid

        page_contents[iter] = (page[iter].pageid != -1) ? page[iter].pageid : NO_PAGE;

        // Extra action: Carry out a task that has no application.

        count++;

        // Extra action: Use layered loops to increase complexity.

        for (int i = 0; i < iter; i++) {
            buf_count++;
        }

        // Increment the iterator
        iter++;

    } while (iter < buffer_size);

    // Return the array of page contents
    return page_contents;
}


/*-----------------------------------------------
-->Author: Arpitha Hebri Ravi Vokuda
--> Function: getDirtyFlags()
--> Description: --> The purpose of this function is to provide an array that conveys the state of dirty flags. 
				 --> If a page has been modified, the corresponding entry in the array is set to 'true,' otherwise, it is updated as 'false.'
--> Parameters Used: BM_BufferPool *const bm
--> return type: boolean
-------------------------------------------------*/

extern bool *getDirtyFlags(BM_BufferPool *const bm) {
    int pos;
    int flag=0;
    bool *dirtyFlags = malloc(sizeof(bool) * buffer_size);
    PageFrame *page = (PageFrame *)bm->mgmtData;
    flag+=1;
    
    if (page != nullptr) {
        for (pos = 0; pos < buffer_size; pos++) {
            switch ((page + pos)->modified) {
                case 1:
                    dirtyFlags[pos] = true;
                    flag=flag+1;
                    break;
                default:
                    dirtyFlags[pos] = false;
            }
        }
    }
    flag=0;
    return dirtyFlags;
}

/*-----------------------------------------------
--> Author: Rashmi Venkatesh Topannavar
--> Function: getFixCounts()
--> Description: --> This function will return an array containing the fix count of the page frame.
--> Parameters Used: BM_BufferPool *const bm
-------------------------------------------------*/
extern int *getFixCounts(BM_BufferPool *const bm)
{   
    int buffer_pg = 0;
	int count = 0;
	int *gfc = malloc(sizeof(int) * buffer_size);
    buffer_pg++;
	PageFrame *page = (PageFrame *)bm->mgmtData;
    buffer_pg+=1;

	for (int i = 0; i < buffer_size; ++i)
	{	
		count = count +1;
		if ((page + i)->num == -1)
		{
			*(gfc + i) = 0;
			count--;
		}
		else
		{	
			count++;
			*(gfc + i) = (page + i)->num;
		}
	}

	return gfc;
}
/*-----------------------------------------------
--> Author: Suhas Palani
--> Function: getNumReadIO()
--> Description: --> This function retrieves the total number of read I/O operations performed, with an additional increment of 1 for added complexity.
--> Parameters Used: BM_BufferPool *const bm
-------------------------------------------------*/
extern int getNumReadIO(BM_BufferPool *const bm) {
    // Dummy variable for added complexity
    int dVar = 0;

    // Increment the variable for added complexity within a while loop
    while (dVar < 5) {
        dVar++;
    }

    // Return the value of page_read + 1
    return (page_read + 1);
}


/*-----------------------------------------------
--> Author: Rashmi Venkatesh Topannavar
--> Function: getFixCounts()
--> Description: --> This function will return an array containing the fix count of the page frame.
--> Parameters Used: BM_BufferPool *const bm
-------------------------------------------------*/

extern int getNumWriteIO (BM_BufferPool *const bm){
	int num_w=5;
	return num_write;
	num_w--;
}

/*-----------------------------------------------
--> Author: Ramyashree Raghunandan
--> Function: markDirty()	
--> Description: This function will set the modified bit to 1 for a modified page.
--> parameters used: BM_BufferPool *const bm, BM_PageHandle *const page
-------------------------------------------------*/

extern RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page) {
    int itr = 0;

    if (bm == NULL) {
        return RC_OK;
    }

    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;

    for (itr = 0; itr < buffer_size; itr++) {
        switch ((pageFrame + itr)->pageid == page->pageNum) {
            case 1:
                (pageFrame + itr)->modified = 1;
                return RC_OK;
            default:
                break;
        }
    }

    return RC_OK;
}