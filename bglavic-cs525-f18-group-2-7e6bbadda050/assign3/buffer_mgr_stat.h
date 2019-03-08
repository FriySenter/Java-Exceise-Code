
#ifndef buffer_mgr_stat_h
#define buffer_mgr_stat_h

#include <stdio.h>
#include "buffer_mgr.h"

// debug functions
void printPoolContent (BM_BufferPool *const bm);
void printPageContent (BM_PageHandle *const page);
char *sprintPoolContent (BM_BufferPool *const bm);
char *sprintPageContent (BM_PageHandle *const page);

#endif /* buffer_mgr_stat_h */
