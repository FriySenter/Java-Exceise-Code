#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

// Include return codes and methods for logging errors
#include "dberror.h"
#include "storage_mgr.h"

// Include bool DT
#include "dt.h"

// Replacement Strategies
typedef enum ReplacementStrategy {
  RS_FIFO = 0,
  RS_LRU = 1,
  RS_CLOCK = 2,
  RS_LFU = 3,
  RS_LRU_K = 4
} ReplacementStrategy;

// Data Types and Structures
typedef int PageNumber;
#define NO_PAGE -1


typedef struct BM_PageHandle {
  PageNumber pageNum;
  char *data;
} BM_PageHandle;


//defination of BufferFrame
typedef struct bufferFrame{
    BM_PageHandle *bm_ph; //target data
    int BF_totalNumber;
    int BF_currentPageNumber;
    int BF_fixNum;
    bool BF_status; //inuse or aviluable?
    bool BF_dirty; //dirty or not?
    int BF_FIFO_count; //FIFO parameter
    int BF_LRU_count; //LRT parameter
} bufferFrame;

//defination of BM_mgmtData
typedef struct BM_mgmtData{
    bufferFrame *BufferPool;
    SM_FileHandle *fHandle;
    int bm_read_count;
    int bm_write_count;
}BM_mgmtData;

typedef struct BM_BufferPool {
    char *pageFile;
    int numPages;
    ReplacementStrategy strategy;
    BM_mgmtData *mgmtData; // use this one to store the bookkeeping info your buffer
    // manager needs for a buffer pool
} BM_BufferPool;


// convenience macros
#define MAKE_POOL()					\
  ((BM_BufferPool *) malloc (sizeof(BM_BufferPool)))

#define MAKE_PAGE_HANDLE()				\
  ((BM_PageHandle *) malloc (sizeof(BM_PageHandle)))

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		  const int numPages, ReplacementStrategy strategy, 
		  void *stratData);
RC shutdownBufferPool(BM_BufferPool *const bm);
RC forceFlushPool(BM_BufferPool *const bm);

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page);
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page);
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page);
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum);

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm);
bool *getDirtyFlags (BM_BufferPool *const bm);
int *getFixCounts (BM_BufferPool *const bm);
int getNumReadIO (BM_BufferPool *const bm);
int getNumWriteIO (BM_BufferPool *const bm);

#endif
