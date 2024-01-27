#include "dataStructure.h"

void FIFOStrategy(BM_BufferPool *const bm, BM_BufferPoolManager *bmgr) {
    int numPages = bm->numPages;
    int currentIndex = readIO % numPages;
    int lastIndex = currentIndex % numPages;
    int pageIndex = 0;
    SM_FileHandle fHandle;

    BM_BufferPoolManager *tempBmgr = (BM_BufferPoolManager * )bm->mgmtData;

    while(pageIndex < numPages){
        if(tempBmgr[currentIndex].fixCount == 0){

            bool isDirtyFlag = bmgr->isDirtyFlag;
            int fixCount = bmgr->fixCount;
            SM_PageHandle data = bmgr->data;    
            PageNumber pageSerial = bmgr->pageSerial;

            if(tempBmgr[currentIndex].isDirtyFlag == true){
                RC fileOpenStatus = openPageFile(bm->pageFile, &fHandle);
                if (fileOpenStatus != RC_OK) {
                    RC message = RC_READ_NON_EXISTING_PAGE;
                }
                else {
                    RC writeResult = writeBlock(tempBmgr[currentIndex].pageSerial, &fHandle, tempBmgr[currentIndex].data);
                    if(writeResult == RC_OK){
                        writeIO++;
                    }
                }
            }

             printf("success point");
            tempBmgr[currentIndex].isDirtyFlag = isDirtyFlag;
            printf("success point");
            tempBmgr[currentIndex].fixCount = fixCount;
            printf("success point");
            tempBmgr[currentIndex].data = data;
            printf("success point");
            tempBmgr[currentIndex].pageSerial = pageSerial;
            break;
        }
        else {
            currentIndex++;
            if(lastIndex == 0){
                currentIndex = 0;
            }
            else{
                currentIndex = currentIndex;
            }
        }
        pageIndex++;
    };
}
