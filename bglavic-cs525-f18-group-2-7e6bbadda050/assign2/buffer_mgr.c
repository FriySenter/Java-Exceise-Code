#include "buffer_mgr_stat.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//************** local help ***************//
#define RC_NULL_POINTER -1
#define RC_NON_FUNCTION -2
#define RC_NEED_UNPIN -3

#define CHECK_WRITE(int)           \
do{                         \
if(int != RC_OK){               \
THROW(RC_WRITE_FAILED, "WRITE_FAILED");          \
}                       \
}while(0);              \

#define CHECK_POINTER(ptr)          \
do{                     \
if(ptr == NULL){            \
THROW(RC_NULL_POINTER, "NULL_POINTER");         \
}                   \
}while(0);              \

#define SET_NULL(ptr)           \
do{         \
if(ptr != NULL){        \
ptr = NULL;             \
}               \
}while(0);          \

int write_flag;     //check the return value for writeblock function
int temp_FIFO = 1;      //FIFO counter
int temp_LRU = 1;       //LRU conter

/**************** BUFFER POOL FUNCTIONS ****************/

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData){
    
    //initialization fHandle
     SM_FileHandle *filehandle = calloc(1, sizeof(SM_FileHandle));
    int returncode = openPageFile((char*) pageFileName, filehandle);
    
    //check if the file exist
    if(returncode != RC_OK){
        free(filehandle);
        THROW(RC_READ_NON_EXISTING_PAGE, "INITIALIZATION: PAGE_FILE NON EXIST!");
    }
    
    //initialization mgmtData
    BM_mgmtData* mgmtData = calloc(1, sizeof(BM_mgmtData));
    mgmtData->bm_write_count = 0;
    mgmtData->bm_read_count = 0;
    mgmtData->fHandle = filehandle;
    mgmtData->BufferPool = calloc(numPages, sizeof(bufferFrame));
    
    //initialization buffer frame
    for(int i = 0; i < numPages; i++){
        //every frame should be empty
        bufferFrame* bf = &mgmtData->BufferPool[i];
        bf->BF_totalNumber = numPages;
        bf->BF_currentPageNumber = i;
        bf->bm_ph = calloc(1, sizeof(BM_PageHandle));
        bf->bm_ph->data = calloc(PAGE_SIZE, sizeof(char));
        bf->bm_ph->pageNum = NO_PAGE;
        
        bf->BF_fixNum = 0;
        bf->BF_status = FALSE;
        bf->BF_dirty = FALSE;
        bf->BF_FIFO_count = 0;
        bf->BF_LRU_count = 0;
    }
    
    //give info to bm
    bm->numPages = numPages;
    bm->pageFile = (char*)pageFileName;
    bm->strategy = strategy;
    bm->mgmtData = mgmtData;
    
    return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm){
    CHECK_POINTER(bm);
    
    //check fix number
    for(int i = 0; i < bm->numPages; i++){
        bufferFrame* bf = &bm->mgmtData->BufferPool[i];
        if(bf->BF_fixNum != 0){
            THROW(RC_NEED_UNPIN, "UNPIN_PAGE_NEEDED");
            return RC_NEED_UNPIN;
        }
    }
    
    //write data into file
    write_flag = forceFlushPool(bm);
    CHECK_WRITE(write_flag);
    
    //free buffer pool
    bm->mgmtData->fHandle = NULL;
    for(int i = 0; i < bm->numPages; i++){
        bufferFrame* bf = &bm->mgmtData->BufferPool[i];
        
            free(bf->bm_ph);
            SET_NULL(bf->bm_ph);
        
    }
    free(bm->mgmtData->BufferPool);
    SET_NULL(bm->mgmtData->BufferPool);
    free(bm->mgmtData);
    SET_NULL(bm->mgmtData);
    
    
    return RC_OK;
}

RC forceFlushPool(BM_BufferPool *const bm){
    // check null
    CHECK_POINTER(bm);
    
    //write dirty file into disk
    for(int i = 0; i < bm->numPages; i++){
        bufferFrame* bf = &bm->mgmtData->BufferPool[i];
        if(bf->BF_dirty == TRUE && bf->BF_fixNum == 0){
           write_flag = forcePage(bm, bf->bm_ph);
            CHECK_WRITE(write_flag);
        }
    }
    
    return RC_OK;
}

/**************** PAGE MANAGEMENT FUNCTIONS ****************/

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
    //check
    CHECK_POINTER(bm);
    CHECK_POINTER(page);
    
    //ensure capacity
    ensureCapacity(pageNum, bm->mgmtData->fHandle);
    
    int target_page = 0;
    
    //read data into buffer frame
    for(int i = 0; i < bm->numPages; i++){
        //buffer not full
        bufferFrame* bf = &bm->mgmtData->BufferPool[i];
        //check if the page already read in the buffer pool
        if(bf->BF_status == TRUE && bf->bm_ph->pageNum == pageNum){
            //return pointer point to the buffer pool
            page->pageNum = pageNum;
            page->data = bf->bm_ph->data;
            bf->BF_fixNum++;
            bf->BF_LRU_count = temp_LRU;
            temp_LRU++;
            break;
        }
        
        //page not in buffer pool need to read from disk
        //find an empty buffer page
        if(bf->BF_status == FALSE){
            bf->bm_ph->pageNum = pageNum;
            //read data from disk
            readBlock(pageNum, bm->mgmtData->fHandle, bf->bm_ph->data);
            bf->BF_status = TRUE;
            bf->BF_fixNum++;
            bm->mgmtData->bm_read_count++;
            bf->BF_FIFO_count = temp_FIFO;
            bf->BF_LRU_count = temp_LRU;
            temp_FIFO = temp_FIFO + 1;
            temp_LRU++;
            
            page->pageNum = bf->bm_ph->pageNum;
            page->data = bf->bm_ph->data;
            
            break;
        }
        
        //buffer is full need replacement
        if(i == bm->numPages - 1){
            int min_FIFO = bm->mgmtData->BufferPool[0].BF_FIFO_count;
            int min_LRU = bm->mgmtData->BufferPool[0].BF_LRU_count;
            switch (bm->strategy) {
                case RS_FIFO:
                    
                    //find the min FIFO page
                    for(int j = 0; j < bm->numPages; j++){
                        bufferFrame* bf = &bm->mgmtData->BufferPool[j];
                        if(bf->BF_FIFO_count < min_FIFO){
                            min_FIFO = bf->BF_FIFO_count;
                            target_page = j;
                        }
                        
                    }
                
                    break;
                case RS_LRU:
                    
                    //find the min LRU count
                    for(int j = 0; j < bm->numPages; j++){
                        bufferFrame* bf = &bm->mgmtData->BufferPool[j];
                        if(bf->BF_LRU_count < min_LRU){
                            min_LRU = bf->BF_LRU_count;
                            target_page = j;
                        }
                        
                    }
                    break;
                default:
                    
                    THROW(RC_NON_FUNCTION, "NO_SUCH_FUNTION!");
                    break;
            }
            
            //find target_page
            for(int i = 0; i < bm->numPages; i++){
                bufferFrame* bf = &bm->mgmtData->BufferPool[i];
                //check target_page fixNum
                if(i == target_page && bf->BF_fixNum == 0){
                //check dirty or not
                if(bf->BF_dirty == FALSE){
                    //replace this buffer frame
                    bf->bm_ph->pageNum = pageNum;
                    readBlock(pageNum, bm->mgmtData->fHandle, bf->bm_ph->data);
                    bf->BF_status = TRUE;
                    bf->BF_fixNum++;
                    bm->mgmtData->bm_read_count++;
                    bf->BF_FIFO_count = temp_FIFO;
                    bf->BF_LRU_count = temp_LRU;
                    temp_FIFO = temp_FIFO + 1;
                    temp_LRU++;
                    
                    page->pageNum = bf->bm_ph->pageNum;
                    page->data = bf->bm_ph->data;
                    break;
                    
                }else{
                    //buffer frame is dirty, write to disk
                    write_flag = forcePage(bm, bf->bm_ph);
                    CHECK_WRITE(write_flag);
                    //read from disk
                    bf->bm_ph->pageNum = pageNum;
                    readBlock(pageNum, bm->mgmtData->fHandle, bf->bm_ph->data);
                    bf->BF_status = TRUE;
                    bf->BF_fixNum++;
                    bm->mgmtData->bm_read_count++;
                    bf->BF_FIFO_count = temp_FIFO;
                    bf->BF_LRU_count = temp_LRU;
                    temp_LRU++;
                    temp_FIFO = temp_FIFO + 1;
                    
                    page->pageNum = bf->bm_ph->pageNum;
                    page->data = bf->bm_ph->data;
                    break;
                    }

                }
                
                //target_page fixNum != 0 find the next page which finNum == 0
                if(i == target_page && bf->BF_fixNum != 0){
                    for(int j = 0; j < bm->numPages; j++){
                        bufferFrame* bf = &bm->mgmtData->BufferPool[j];
                        
                        if(bf->BF_FIFO_count == min_FIFO + 1 && bf->BF_fixNum == 0){
                            //check dirty or not
                            if(bf->BF_dirty == FALSE){
                                //replace this buffer frame
                                bf->bm_ph->pageNum = pageNum;
                                readBlock(pageNum, bm->mgmtData->fHandle, bf->bm_ph->data);
                                bf->BF_status = TRUE;
                                bf->BF_fixNum++;
                                bm->mgmtData->bm_read_count++;
                                bf->BF_FIFO_count = temp_FIFO;
                                bf->BF_LRU_count = temp_LRU;
                                temp_FIFO = temp_FIFO + 1;
                                temp_LRU++;
                                
                                page->pageNum = bf->bm_ph->pageNum;
                                page->data = bf->bm_ph->data;
                                break;
                                
                            }else{
                                //buffer frame is dirty, write to disk
                                write_flag = forcePage(bm, bf->bm_ph);
                                CHECK_WRITE(write_flag);
                                //read from disk
                                bf->bm_ph->pageNum = pageNum;
                                readBlock(pageNum, bm->mgmtData->fHandle, bf->bm_ph->data);
                                bf->BF_status = TRUE;
                                bf->BF_fixNum++;
                                bm->mgmtData->bm_read_count++;
                                bf->BF_FIFO_count = temp_FIFO;
                                bf->BF_LRU_count = temp_LRU;
                                temp_LRU++;
                                temp_FIFO = temp_FIFO + 1;
                                
                                page->pageNum = bf->bm_ph->pageNum;
                                page->data = bf->bm_ph->data;
                                break;
                            }
                        }
                    }
                    
                }
            }
        }
        
    }
    return RC_OK;
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
    CHECK_POINTER(bm);
    
    //find specific page
    for(int i = 0; i < bm->numPages; i++){
        bufferFrame* bf = &bm->mgmtData->BufferPool[i];
        if(bf->bm_ph->pageNum == page->pageNum){
            bf->BF_fixNum = bf->BF_fixNum - 1;
            break;
        }
    }
    return RC_OK;
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
    //check
    CHECK_POINTER(bm);
    
    //find specific page
    for(int i = 0; i < bm->mgmtData->BufferPool->BF_totalNumber; i++){
        bufferFrame* bf = &bm->mgmtData->BufferPool[i];
        if(bf->bm_ph->pageNum == page->pageNum){
            write_flag = writeBlock(bf->bm_ph->pageNum, bm->mgmtData->fHandle, bf->bm_ph->data);
            
            //chcek write status
            if(write_flag == RC_OK){
                bf->BF_dirty = FALSE;
                bm->mgmtData->bm_write_count++;
            }else{
                THROW(RC_WRITE_FAILED, "WRITE_FAILED");
            }
            break;
        }
    }
    
    return RC_OK;
}

/**************** STATISTIC FUNCTIONS ****************/

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
    CHECK_POINTER(bm);
    
    // mark frame dirty
    for(int i = 0; i < bm->numPages; i++){
        bufferFrame* bf = &bm->mgmtData->BufferPool[i];
        if (bf->bm_ph->pageNum == page->pageNum){
            bf->BF_dirty = TRUE;
            break;
        }
    }
    return RC_OK;
}

PageNumber *getFrameContents (BM_BufferPool *const bm){
    PageNumber *result = calloc(bm->numPages, sizeof(int));
    
    for(int i = 0; i < bm->numPages; i++){
        bufferFrame* bf = &bm->mgmtData->BufferPool[i];
        if (bf->BF_status == FALSE){
            result[i] = NO_PAGE;
        }else{
            result[i] = bf->bm_ph->pageNum;
        }
    }
    return result;
}

bool *getDirtyFlags (BM_BufferPool *const bm){
    bool *result = calloc(bm->numPages, sizeof(int));
    
    for(int i = 0;i<bm->numPages; i++){
        bufferFrame* bf = &bm->mgmtData->BufferPool[i];
        if (bf->BF_dirty == false){
            result[i] = false;
        }else{
            result[i] = true;
        }
    }
    return result;
}

int *getFixCounts (BM_BufferPool *const bm){
    int *result = calloc(bm->numPages, sizeof(int));
    
    for(int i = 0;i<bm->numPages ;i++){
        bufferFrame* bf = &bm->mgmtData->BufferPool[i];
        if (bf->BF_status == FALSE){
            result[i] = 0;
        }else{
            result[i] = bf->BF_fixNum;
        }
    }
    return result;
}

int getNumReadIO (BM_BufferPool *const bm){
    return bm->mgmtData->bm_read_count;
}

int getNumWriteIO (BM_BufferPool *const bm){
    return bm->mgmtData->bm_write_count;
}



