//
//  storage_mgr.c
//  CS525HW3-1
//
//  Created by Yudong Wu on 11/4/18.
//  Copyright © 2018 Yudong Wu. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "storage_mgr.h"



#define RC_NULL_POINTER -1
#define BYTE_SIZE 1
#define CHECK_POINTER(ptr)              \
do{                                        \
if(ptr == NULL){                    \
THROW(-1, "NULL POINTER!");     \
}                                   \
}while(0);

#define CHECK_FHANDLES(ptr)          \
do{                                   \
if(ptr == NULL || ptr->mgmtInfo == NULL){                      \
THROW(2, "NULL_FHANDLE!");         \
return RC_FILE_HANDLE_NOT_INIT;     \
}           \
}while(0);

//Global var
long flength;
size_t writeStatus, readStatus;
int closeStatus;
//int BYTE_SIZE = sizeof(char);



void initStorageManager(void){
    //    printf("initial the store manager!");
}


//format a page
RC formatPage(FILE *pageFile){
    //check pointer
    CHECK_POINTER(pageFile);
    
    //set memory & format a page
    int* temp = (int*)malloc(PAGE_SIZE);
    memset(temp, 0, PAGE_SIZE);
    writeStatus = fwrite(temp, BYTE_SIZE, PAGE_SIZE, pageFile);
    //check write status
    if(writeStatus != PAGE_SIZE){
        THROW(3, "RC_WRITE_FAILED");
        return RC_WRITE_FAILED;
    }
    free(temp);
    
    return RC_OK;
}





//create filepage function
RC createPageFile(char* fileName){
    
    //check fileName
    CHECK_POINTER(fileName);
    
    //create one page
    FILE *pageFile = fopen(fileName, "w");
    
    formatPage(pageFile);
    
    //close stream
    closeStatus = fclose(pageFile);
    //check file colse success
    if(closeStatus != 0){
        RC_message = "CLOSE_FILE_FIELD";
    }
    
    return RC_OK;
}

//open page file function
RC openPageFile(char *fileName, SM_FileHandle *fHandles){
    //check pointer
    CHECK_POINTER(fileName);
    CHECK_POINTER(fHandles);
    
    FILE *pageFile = fopen(fileName, "r+");
    if(pageFile == NULL){
        THROW(1, "FILE NOT FOUND!");
        return RC_FILE_NOT_FOUND;
    }
    
    //get file length
    fseek(pageFile, 0L, SEEK_END);
    flength = ftell(pageFile);
    
    //set pointer to the start of the file
    rewind(pageFile);
    
    //initialize fHandles
    fHandles->fileName = fileName;
    fHandles->curPagePos = 0;
    fHandles->totalNumPages = (int)(flength / PAGE_SIZE);
    fHandles->mgmtInfo = pageFile;
    
    return RC_OK;
}

//close file function
RC closePageFile (SM_FileHandle *fHandles){
    //check fHandle
    CHECK_FHANDLES(fHandles);
    
    closeStatus = fclose(fHandles->mgmtInfo);
    //check close status
    if(closeStatus != 0){
        RC_message = "CLOSE_FILE_FIELD!";
    }
    
    return RC_OK;
}


//destoryPage function
RC destroyPageFile(char* fileName){
    CHECK_POINTER(fileName);
    
    //check if the file exist
    FILE* pageFile = fopen(fileName, "r");
    if(pageFile == NULL){
        THROW(1, "FILE NOT FOUND!");
        return RC_FILE_NOT_FOUND;
    }
    closeStatus = fclose(pageFile);
    if(closeStatus != 0){
        RC_message = "CLOSE_FILE_FIELD!";
    }
    
    remove(fileName);
    
    return RC_OK;
}

//read block function
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    //check pointer
    CHECK_POINTER(fHandle);
    CHECK_POINTER(memPage);
    
    //check pageNum
    if(pageNum > fHandle->totalNumPages || pageNum < 0 ){
        //        printf("page does not exist");
        THROW(4, "READ_NON_EXIST_PAGE!");
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    
    //seek to the page
    fseek((FILE*)fHandle->mgmtInfo, pageNum * PAGE_SIZE * BYTE_SIZE , SEEK_SET);
    //update current page
    fHandle->curPagePos = pageNum;
    
    readStatus = fread(memPage, BYTE_SIZE, PAGE_SIZE, fHandle->mgmtInfo);
    if(readStatus != PAGE_SIZE){
        RC_message = "READ_FILED";
    }
    
    return RC_OK;
}

//get block position
int getBlockPos(SM_FileHandle *fHandle){
    //check fHandll
    CHECK_FHANDLES(fHandle);
    
    return fHandle->curPagePos;
}

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //check fHandle
    CHECK_FHANDLES(fHandle);
    
    //read
    return readBlock(0, fHandle, memPage);
}

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //chcek fHandle
    CHECK_FHANDLES(fHandle);
    
    //check if has a previous block
    if(fHandle->curPagePos == 0 ){
        RC_message = "THIS IS THE FIRST BLOCK!";
        THROW(4, "READ_NON_EXIST_PAGE");
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    //read
    return readBlock(fHandle->curPagePos - 1, fHandle, memPage);
}

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //check fHandle
    CHECK_FHANDLES(fHandle);
    
    //read
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //check fhandle
    CHECK_FHANDLES(fHandle);
    
    //check if this is the last block
    if(fHandle->curPagePos >= fHandle->totalNumPages - 1){
        RC_message = "THIS IS THE LAST BLOCK!";
        THROW(4, "READ_NON_EXIST_PAGE");
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    //read
    return readBlock(fHandle->curPagePos + 1, fHandle, memPage);
}


RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //check fHandle
    CHECK_FHANDLES(fHandle);
    
    //read
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    //check fHandle
    CHECK_FHANDLES(fHandle);
    CHECK_POINTER(memPage);
    
    //check pageNum
    if(pageNum > fHandle->totalNumPages || pageNum < 0 ){
        appendEmptyBlock(fHandle);
//        THROW(4, "READ_NON_EXIST_PAGE!");
//        return RC_READ_NON_EXISTING_PAGE;
    }
    
    //seek to the position
    fseek((FILE*)fHandle->mgmtInfo, pageNum * PAGE_SIZE * BYTE_SIZE , SEEK_SET);
    writeStatus = fwrite(memPage, BYTE_SIZE, PAGE_SIZE, fHandle->mgmtInfo);
    if(writeStatus != PAGE_SIZE){
        THROW(-1, "WRITE FAILED!");
    }
    
    return RC_OK;
}

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //check fHandle
    CHECK_FHANDLES(fHandle);
    
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

RC appendEmptyBlock (SM_FileHandle *fHandle){
    //check fHandle
    CHECK_FHANDLES(fHandle);
    
    //seek to the end of the page
    fseek(fHandle->mgmtInfo, 0, SEEK_END);
    //add a blank page
    formatPage(fHandle->mgmtInfo);
    //update total page number & current page
    fHandle->totalNumPages ++;
    fHandle->curPagePos ++;
    return RC_OK;
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    //check if current file satisfied the number
    if(fHandle->totalNumPages >= numberOfPages){
        //        printf("already have enough room!");
        return RC_OK;
    }
    
    for (int i = 0; i < numberOfPages - fHandle->totalNumPages; i++) {
        RC rc = appendEmptyBlock(fHandle);
        if (rc != RC_OK)
            return rc;
    }
    //    int* temp = (int*)malloc(PAGE_SIZE * (numberOfPages - fHandle->totalNumPages));
    //    memset(temp, 0, PAGE_SIZE * (numberOfPages - fHandle->totalNumPages));
    //    writeStatus = fwrite(temp, BYTE_SIZE, PAGE_SIZE * (numberOfPages - fHandle->totalNumPages), fHandle->mgmtInfo);
    //    if(writeStatus != PAGE_SIZE * (numberOfPages - fHandle->totalNumPages)){
    //        THROW(-2, "WRITE_FIELD");
    //    }
    //    fHandle->totalNumPages++;
    //    free(temp);
    
    return RC_OK;
}
