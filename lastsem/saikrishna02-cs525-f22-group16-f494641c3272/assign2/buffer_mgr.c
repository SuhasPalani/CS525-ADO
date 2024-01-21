#include<stdio.h>
#include<stdlib.h>
#include <math.h>

#include "buffer_mgr.h"
#include "storage_mgr.h"

/*******************************
 * Author: Bhoomika Panduranga
*******************************/

// This structure represents one page frame in buffer pool (memory).
typedef struct Page
{
    SM_PageHandle page_h; // Data obtained from the page
    PageNumber pageid; // id for the page
    int modified; // Used to indicate whether the contents of the page has been modified by the client
    int num; // Used to indicate the number of clients using that page at a given instance
    int lru_num;   // Used by LRU algorithm to get the least recently used page	
    int lfu_num;   // Used by LFU algorithm to get the least frequently used page
} PageFrame;

/*******************************
 * Author: Bhoomika Panduranga
*******************************/

// initializing all variables and pointers we use to zero 
// "buffer_size" setting the size of buffer pool to 0
// "page_read" number of pages read.
// "page_read" is also used by FIFO function to calculate the frontIndex i.e.
// "writeCount" number if writes done to the disk 
// "index_hit" a general count which is incremented whenever a page frame is added into the buffer pool.
// "index_hit" is used by LRU to determine least recently added page into the buffer pool.
// "clock_index" used in clock replacement algorithm. Represents last page added.
// "lfu_index" used as pointer to store least freq used page frames
int buffer_size,page_read,num_write,index_hit,clock_index,lfu_index= 0;



void copyPageFrames(PageFrame *dest, int index,PageFrame *src){
	
	(dest+index)->page_h = src->page_h; 
	(dest+index)->modified = src->modified;
	(dest+index)->num = src->num;
	(dest+index)->pageid= src->pageid;
	
	return;

}

void writePageFrames (BM_BufferPool *const bp,  PageFrame *page_f, int page_index )
{
	SM_FileHandle f_handle;
	openPageFile(bp->pageFile, &f_handle);
	writeBlock(page_f[page_index].pageid, &f_handle, page_f[page_index].page_h); 
	num_write=1+num_write;
	
	return;

}

/*******************************
 * Author: Bhoomika Panduranga
*******************************/
// Implementation of FIFO (First In First Out) algorithm
extern void FIFO(BM_BufferPool *const bp, PageFrame *pf)
{
	int page_index = page_read % buffer_size;
    PageFrame *page_f= (PageFrame *) bp->mgmtData;
	for(int j = 0; j < buffer_size; j++){    // performing iteration for the existing page frames of buffer pool
		if(page_f[page_index].num != 0){
			page_index = page_index + 1;
			page_index = (page_index % buffer_size == 0) ? 0 : page_index; // move to the next location if current frame is being used
		}
		else{
			if((page_f+page_index)->modified == 1){ 
			//  write page to disk and set modified to one if the page is modified
			writePageFrames(bp,page_f,page_index);
            }
            //making the new page content same as page frame's
            copyPageFrames(page_f,page_index,pf);	
			break;
		}
	}
}


/*******************************
 * Author: Bhoomika Panduranga
*******************************/

// Implementation of LRU (Least Recently Used) algorithm
extern void LRU(BM_BufferPool *const bp, PageFrame *pf)
{	
	int j=0;
    int index =0;
    int least_number=0;
    PageFrame *page_f ;

    if(bp != NULL){
        page_f = (PageFrame *)bp->mgmtData;
    }
	for(j = 0; j < buffer_size; j++){ // performing iteration for the existing page frames of buffer pool
		if((page_f+j)->num == 0){ // Identifying the page frame which is free
            least_number = (page_f+j)->lru_num;
			index = j;
			break;
		}
	}	
	for(j = index + 1; j < buffer_size; j++){ // identifying least recently used page frames
		if(least_number>(page_f+j)->lru_num){
			least_number = (page_f+j)->lru_num;
            index = j;
		}
	}
	if((page_f+index)->modified == 1){ //  write page to disk and set modified to one if the page is modified
		writePageFrames(bp,page_f,index);
		// increasing the num of write after write operation
	}
	//making the new page content same as page frame's
   copyPageFrames(page_f,index,pf);
    page_f[index].lru_num=pf->lru_num;
}


extern void LRU_K(BM_BufferPool *const bp, PageFrame *pf)
{	
	PageFrame *page_f = (PageFrame *) bp->mgmtData;
	int j, index, least_number;
	for(j = 0; j < buffer_size; j++){ // performing iteration for the existing page frames of buffer pool
		if((page_f+j)->num == 0){ // Identifying the page frame which is free
            least_number = (page_f+j)->lru_num;
			index = j;
			break;
		}
	}	
	for(j = index + 1; j < buffer_size; j++){ // identifying least recently used page frames
		if(least_number>(page_f+j)->lru_num){
			least_number = (page_f+j)->lru_num;
            index = j;
		}
	}
	if((page_f+index)->modified == 1){ //  write page to disk and set modified to one if the page is modified
		writePageFrames(bp,page_f,index);

	}
	//making the new page content same as page frame's
   	copyPageFrames(page_f,index,pf);
	(page_f+index)->lru_num = pf->lru_num;
}

/*******************************
 * Author: Bhoomika Panduranga
*******************************/

// Defining CLOCK function
extern void CLOCK(BM_BufferPool *const bp, PageFrame *pf)
{
	PageFrame *page_f ;
	if(bp!= NULL){
	page_f =(PageFrame *) bp->mgmtData;
 }
	while(1){
		if (clock_index % buffer_size != 0) {
            clock_index = clock_index;
        } 
        else{
            clock_index = 0;
        }
		if(page_f[clock_index].lru_num != 0)
			page_f[1+ clock_index].lru_num = 0;// finding next frame location
		else{	
            if(page_f[clock_index].modified == 1){ //write page to disk and set modified to one if the page is modified
				writePageFrames(bp,page_f,clock_index);
			}
			//making the new page content same as page frame's
			copyPageFrames(page_f,clock_index,pf);
            page_f[clock_index].lru_num=pf->lru_num;
			clock_index=1+clock_index;
			break;		
		}
	}
}

/**************************
 * Author: Diksha Sharma
**************************/

//This function creates and initializes a buffer pool containing pageid and page frames.
//pg_FName stores the name of the page file whose pages are cached in memory.
//approach represents the page replacement strategy (FIFO, LRU, LFU, CLOCK) that will be used by this buffer pool
//approachData is used to pass parameters if any to the page replacement strategy

extern RC initBufferPool(BM_BufferPool *const bp, const char *const pg_FName, const int p_id,
                         ReplacementStrategy approach, void *approachData)
{
    // Reserver memory space = number of pages x space required for one page
    PageFrame *page_f= malloc(sizeof(PageFrame) *p_id);
    int i=0;
    // Buffersize is the total number of pages in memory or the buffer pool.
	buffer_size = p_id;	
	bp->pageFile = (char *)pg_FName;
	bp->numPages = p_id;
	bp->strategy = approach;
	while (i<buffer_size)
	{
		
		(page_f+i)->page_h = NULL;
		(page_f+i)->pageid = -1;
		(page_f+i)->modified = 0;
		(page_f+i)->num = 0;
		(page_f+i)->lru_num = 0;
		(page_f+i)->lfu_num = 0;
		i++;
	}
	bp->mgmtData=page_f;
	clock_index = 0;
	lfu_index = 0;
	num_write = 0;
	
	return RC_OK;	
}

/**************************
 * Author: Diksha Sharma
**************************/

// This function writes all the dirty pages (having fixCount = 0) to disk

extern RC forceFlushPool(BM_BufferPool *const bp)
{
	int index=0;
	PageFrame * page_f= (PageFrame *)bp->mgmtData;
	// Store all dirty pages (modified pages) in memory to page file on disk	
    while(index < buffer_size)
    {
       if(page_f[index].modified == 1 && page_f[index].num == 0)
		{
			writePageFrames(bp, page_f,index);
			(page_f+index)->modified = 0;
		}
		index++;
    }
 return RC_OK;	
}


/**************************
 * Author: Diksha Sharma
**************************/

extern RC shutdownBufferPool(BM_BufferPool *const bp)
{
	PageFrame *page_f = (PageFrame *)bp->mgmtData;
	// Write all dirty pages (modified pages) back to disk
	forceFlushPool(bp);
	// Releasing space occupied by the page
	free(page_f);
	int index=0;
	// Store all dirty pages (modified pages) in memory to page file on disk	
    while(index < buffer_size)
    {
       if(page_f[index].modified == 1 && page_f[index].num == 0)
		{
			writePageFrames(bp, page_f,index);
			(page_f+index)->modified = 0;
		}
		index++;
    }
	bp->mgmtData = NULL;
	return RC_OK;
}



// ***** PAGE MANAGEMENT FUNCTIONS ***** //

/**************************
 * Author: Diksha Sharma
**************************/
extern RC unpinPage (BM_BufferPool *const bp, BM_PageHandle *const pg)
{	
	int index=0;
	PageFrame *page_f = (PageFrame *)bp->mgmtData;
	// Iterating through all the pages in the buffer pool
	while (index<buffer_size)
	{
		// If the current page is the page to be unpinned, then decrease fixCount (which means client has completed work on that page) and exit loop
		if(page_f[index].pageid == pg->pageNum)
		{
			(page_f+index)->num--;
			break;		
		}
	  index++;
	}
	return RC_OK;
}

/**************************
 * Author: Diksha Sharma
**************************/

//writes the contents of the modified pages back to the page file on disk
extern RC forcePage (BM_BufferPool *const bp, BM_PageHandle *const pg)
{
	PageFrame *page_f = (PageFrame *)bp->mgmtData;
	
	int index=0;
	// Iterating through all the pages in the buffer pool
	while (index < buffer_size)
	{
		// If the current page = page to be written to disk, then right the page to the disk using the storage manager functions
		if(pg->pageNum == page_f[index].pageid)
		{		
			writePageFrames(bp, page_f,index);
			(page_f+index)->modified = 0;
		}
	  index++;
	}
	return RC_OK;	
}

/*******************************
 * Author: Bhoomika Panduranga
*******************************/

extern RC pinPage (BM_BufferPool *const bp, BM_PageHandle *const p_handle, const PageNumber pageid)
{
	PageFrame *page_f = (PageFrame *)bp->mgmtData;
	SM_FileHandle f_handle;
	// This is the first page to be pinned, therefore I'm checking if the buffer pool is empty.

	if(page_f[0].pageid != -1)
	{
		bool buffer_size_full = true;
		for(int j = 0; j < buffer_size; j++){
			if(page_f[j].pageid != -1){		
				if((page_f+j)->pageid == pageid){  // Checking if page is in memory
					(page_f+j)->num++;
					buffer_size_full = false;
					
					index_hit = index_hit + 1; // Incrementing count
					if(bp->strategy == RS_LRU)
					(page_f+j)->lru_num = index_hit; // Identify the least recently used page through LRU alg by sending count.
					else if(bp->strategy == RS_CLOCK)
						(page_f+j)->lru_num = 1;	// to make the final page frame
					(*p_handle).data = (page_f+j)->page_h;
					(*p_handle).pageNum = pageid;
					clock_index = clock_index + 1;
					break;
				}				
			} 
			else {
				openPageFile(bp->pageFile, &f_handle);
				page_f[j].page_h = (SM_PageHandle) malloc(PAGE_SIZE);
				readBlock(pageid, &f_handle, page_f[j].page_h);
				(page_f+j)->num = 1;
				(page_f+j)->pageid = pageid;
				(page_f+j)->lfu_num = 0;
				page_read =	page_read +1;
				index_hit =	index_hit +1;
				if(bp->strategy == RS_LRU) 
				(page_f+j)->lru_num = index_hit;	// here LRU algorithm determines least recently used page based on count
				else if(bp->strategy == RS_CLOCK)
				(page_f+j)->lru_num = 1;  // final page frame
				buffer_size_full = false;
				(*p_handle).pageNum = pageid;
				(*p_handle).data = page_f[j].page_h;
				break;
			} 	
		}
		if(buffer_size_full == true){
			PageFrame *page_new = (PageFrame *) malloc(sizeof(PageFrame));	// new page to store data read from the file.
			openPageFile(bp->pageFile, &f_handle);
			page_new->page_h = (SM_PageHandle) malloc(PAGE_SIZE);
			readBlock(pageid, &f_handle, page_new->page_h);
			(*page_new).pageid = pageid;		
			(*page_new).num = 1;
			(*page_new).modified= 0;
			(*page_new).lfu_num = 0;
			index_hit =	index_hit +1;
			page_read =	page_read +1;
			if(bp->strategy == RS_LRU)
				page_new->lru_num = index_hit;				
			else if(bp->strategy == RS_CLOCK)
				page_new->lru_num = 1;
			   (*p_handle).pageNum = pageid;
           		(*p_handle).data = page_new->page_h;			           
            // Depending on the page replacement approach chosen, call the relevant algorithm's function (passed through parameters)
            if(bp->strategy == RS_FIFO){            
                
                    FIFO(bp, page_new);
            }else if(bp->strategy== RS_LRU){
                
                    LRU(bp, page_new);
            }else if(bp->strategy==RS_CLOCK){
                    CLOCK(bp, page_new);
            }else if(bp->strategy==RS_LRU_K)    {
                    printf("\n LRU-k algorithm not implemented exactly, but LRU is tested in first test file 'test_assign2_1' correctly\n\n\n");
                    LRU_K(bp, page_new);
            }
                else{
                    printf("\nAlgorithm Not Implemented\n");
                    
            }
		}	
		    return RC_OK;
	}
	else{	
		SM_FileHandle f_handle;
		openPageFile(bp->pageFile, &f_handle);
		(page_f+0)->page_h = (SM_PageHandle) malloc(PAGE_SIZE);
		ensureCapacity(pageid,&f_handle);
		readBlock(pageid, &f_handle, page_f[0].page_h);
		(page_f+0)->pageid = pageid;
		page_read= index_hit = 0;
		(page_f+0)->lfu_num = 0;
		(page_f+0)->num++;	
		(page_f+0)->lru_num = index_hit;
		(*p_handle).pageNum = pageid;
		(*p_handle).data = page_f[0].page_h;
		return RC_OK;		
	}	
}


/*******************************
 * Author: Yashas Shashidhar
*******************************/
extern PageNumber *getFrameContents (BM_BufferPool *const bm){
    
    PageNumber *contents = malloc(sizeof(PageNumber) * buffer_size);
    PageFrame *page = (PageFrame *) bm->mgmtData;
    int itr = 0;
    // Iterating through all of the pages in the buffer pool and updating the value of frame contents to the pageNum.
    while(itr < buffer_size) {
        
        if((page+itr)->pageid != -1 ){
            *(contents + itr) = (page+itr)->pageid;
        }
        else {
            *(contents + itr) = NO_PAGE;
        }
        itr++;
        
    }
    return contents;
}


extern bool *getDirtyFlags (BM_BufferPool *const bm){

    int itr =0;
    bool *dirty_flags = malloc(sizeof(bool) * buffer_size);
    PageFrame *page = (PageFrame *)bm->mgmtData;
    // Iterating through all the pages in the buffer pool and changing dirty_flags' value to TRUE if page is dirty else FALSE
    if(page != NULL){
    do{
        
        if((page+itr)->modified == 1){
            *(dirty_flags + itr) = true;
        }else
            
          dirty_flags[itr] =  false ;
    }while(++itr < buffer_size);            
    }
    
        
    return dirty_flags;

}

/*******************************
 * Author: Yashas Shashidhar
*******************************/
extern int *getFixCounts (BM_BufferPool *const bm){
    
    int itr = 0;
    int *fc = malloc(sizeof(int) * buffer_size);
    PageFrame *page = (PageFrame *)bm->mgmtData;
    
    // Iterating through the buffer pool's pages and setting fix counts to the page's fix count 
    while(itr < buffer_size){
        
        if((page+itr)->num == -1){
            *(fc+itr) = 0;
        }else
        {
            *(fc+itr) = (page+itr)->num;
        }
        
        itr++;
    }    
    return fc;
}

/*******************************
 * Author: Yashas Shashidhar
*******************************/
extern int getNumReadIO (BM_BufferPool *const bm){
	return (page_read + 1);
}

/*******************************
 * Author: Yashas Shashidhar
*******************************/
extern int getNumWriteIO (BM_BufferPool *const bm){
	return num_write;
}

extern RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	int itr =0;

	if(bm != NULL){
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;

	 do{
		if((pageFrame+itr)->pageid == page->pageNum)
	 {
		(pageFrame + itr)->modified = 1;
	 	return RC_OK; 
	 }
	}
	while(++itr < buffer_size);
	}
	return RC_ERROR;
}