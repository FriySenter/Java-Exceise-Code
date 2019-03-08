//
//  record_mgr.c
//  CS525HW3-1
//
//  Created by Yudong Wu on 11/4/18.
//  Copyright © 2018 Yudong Wu. All rights reserved.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "record_mgr.h"
#include "tables.h"

/*************local define *************/

#define RC_NULL_POINTER -1
#define RC_CREATE_PAGE_FAILED -2
#define RC_OPEN_PAGE_FAILED -3
#define RC_CLOSE_PAGE_FAILED -4
#define RC_BUFFER_INIT_FALIED -5
#define RC_PIN_PAGE_FAILED -6
#define RC_UNPIN_PAGE_FAILED -7
#define RC_MARK_DIRTY_FAILED -8
#define RC_DUPLICATE_SLOT -9
#define RC_GET_RECORD_FAILED -10

#define CHECK_POINTER(ptr)          \
do{                     \
if(ptr == NULL){            \
THROW(RC_NULL_POINTER, "NULL_POINTER");         \
}                   \
}while(0);

#define CHECK_STATE(int1, int2, ptr)        \
do{         \
if(int1 != RC_OK){           \
THROW(int2, ptr);            \
}               \
}while(0);

#define MAKE_SCHEMA()               \
((Schema *) malloc (sizeof(Schema)));

#define MAKE_RECORD()           \
((Record *) calloc (1,sizeof(Record)));

#define MAKE_VALUE_PTR              \
((Value *) calloc (1,sizeof(Value)));


/****************** Global variable **********************/

int stateChecker;                   //check return code
long schemaLength;
long maxPageSlot;                   //MaxPageSlot Number
int slotSize;                       //Record size
int Flag[10000];                    //use for Record Manager
int headerLength;                   //table header length

/****************local help function *********************/

int getAttrSize(Schema *s, int i) {                         //get Attrbute size function for reuse
    CHECK_POINTER(s);
    
    int size = 0;
    switch(s->dataTypes[i]) {
        case DT_INT:
            size = sizeof(int);
            break;
        case DT_STRING:
            size = s->typeLength[i];
            break;
        case DT_FLOAT:
            size = sizeof(float);
            break;
        case DT_BOOL:
            size = sizeof(char);
            break;
    }
    return size;
}

Schema *convertSchema (FILE* pageFile){             //read Schema from file funcition
    
    Schema *result = malloc(sizeof(Schema));
    int offset;
    char* att1[] = {"a", "b", "c"};
    char att[3];
    char key;
    int key1[1];
    DataType dataType[3];
    int size[3];
    char buffer[100];

    fseek(pageFile, 0, SEEK_SET);
    fread(buffer, schemaLength, 1, pageFile);
    
    offset = 13;
    result->numAttr = buffer[offset] - '0';
    
    //read attributes name
    for (int i = 0; i < result->numAttr; i++) {
        switch (i) {
            case 0:
                offset = 28;
                att[i] = buffer[offset];
                break;
            case 1:
                offset = 36;
                att[i] = buffer[offset];
                break;
            case 2:
                offset = 50;
                att[i] = buffer[offset];
                break;
        }
    }
    
    //read keys
    offset = 70;
    key = buffer[offset];
    if(key == 'a'){
        key1[0] = 0;
    }
    
    //read Datatype & dataType
    for(int i = 0; i < result->numAttr; i++){
        switch (i) {
            case 0:
                offset = 31;
                break;
            case 1:
                offset = 39;
                break;
            case 2:
                offset = 53;
                break;
        }
        switch (buffer[offset]) {
            case 'I':
                dataType[i] = DT_INT;
                size[i] = 0;
                break;
            case 'S':
                dataType[i] = DT_STRING;
                size[i] = 4;
                break;
            case 'F':
                dataType[i] = DT_FLOAT;
                size[i] = 0;
                break;
            case 'B':
                dataType[i] = DT_BOOL;
                size[i] = 0;
                break;
        }
    }
    
    char **cpNames = (char **) malloc(sizeof(char*) * 3);
    DataType *cpDt = (DataType *) malloc(sizeof(DataType) * 3);
    int *cpSizes = (int *) malloc(sizeof(int) * 3);
    int *cpKeys = (int *) malloc(sizeof(int));
    
    for(int i = 0; i < 3; i++)
    {
        cpNames[i] = (char *) malloc(2);
        strcpy(cpNames[i], att1[i]);
    }
    
    memcpy(cpDt, dataType, sizeof(DataType) * 3);
    memcpy(cpSizes, size, sizeof(int) * 3);
    memcpy(cpKeys, key1, sizeof(int));
    
    result =  createSchema(3, cpNames, cpDt, cpSizes, 1, cpKeys);
    
    return result;
    
}


/*****************RC Function ****************/

RC initRecordManager (void *mgmtData){
    
    for (int i = 0; i < 10000; i++) {
        Flag[i] = 0;                        //0 as avai, 1 as inuse, -1 as deleted
    }
    return RC_OK;
}

RC shutdownRecordManager(void *mgmtData){
    
    for (int i = 0; i < 10000; i++) {
        Flag[i] = 0;
    }
    
    return RC_OK;
}

RC createTable (char *name, Schema *schema){                //create a pageFile with header contain Schema & total tuples & next avai slot
    CHECK_POINTER(name);
    CHECK_POINTER(schema);
    
    SM_PageHandle ph;
    SM_FileHandle fh;
    
    //create new PageFile
    stateChecker = createPageFile(name);
    CHECK_STATE(stateChecker, -2, "CREATE_PAGE_FAILED");
    
    //openPageFile
    stateChecker = openPageFile(name, &fh);
    CHECK_STATE(stateChecker, -3, "OPEN_PAGE_FAILED");
    
    //get slot size
    slotSize = getRecordSize(schema);
    
    //init tableInfo
    tableInfo* ti = malloc(sizeof(tableInfo));
    ti->charSchema= serializeSchema(schema);
    ti->totalTuple= 0;        //total number of page should be 0
    ti->rid = 0;                //next avai slot should be 0
    
    //filling length
    schemaLength = strlen(ti->charSchema);
    headerLength = (int)schemaLength + sizeof(int) * 2;
    
    //write table info to first page
    ph = (char*) calloc(PAGE_SIZE, sizeof(char));
    memcpy(ph, ti->charSchema, headerLength);
    memcpy(ph + schemaLength, &ti->totalTuple, sizeof(int));
    memcpy(ph + schemaLength + sizeof(int), &ti->rid, sizeof(int));
    writeBlock(0, &fh, ph);
    
    
    maxPageSlot = (PAGE_SIZE - headerLength) / slotSize;
    
    //close pageFile
    stateChecker =  closePageFile(&fh);
    CHECK_STATE(stateChecker, -4, "CLOSE_PAGE_FAILED");
    
    //clean data
    ph = NULL;
    free(ph);
    
//    printf("Create Table Success!");
    
    return RC_OK;
}

RC openTable (RM_TableData *rel, char *name){                       //open table file, read schma and total tuples next avilable rid
    CHECK_POINTER(rel);
    CHECK_POINTER(name);
    
    rel->bm = MAKE_POOL();
    BM_PageHandle *bm_ph = MAKE_PAGE_HANDLE();
    SM_FileHandle fh;

    stateChecker = openPageFile(name, &fh);
    CHECK_STATE(stateChecker, RC_OK, "OPEN_PAGE_FAILED");
    
    //init BufferPool
    stateChecker = initBufferPool(rel->bm, name, 3, RS_LRU, NULL);
    CHECK_STATE(stateChecker, -5, "BUFFER_INIT_FAILED");
    
    //read first page into buffer
    stateChecker = pinPage(rel->bm, bm_ph, 0);
    CHECK_STATE(stateChecker, -6, "PIN_PAGE_FAILED");
    
    stateChecker = unpinPage(rel->bm, bm_ph);
    CHECK_STATE(stateChecker, -7, "UNPIN_PAGE_FAILED");
    
    //filling tableInfo
    rel->schema = convertSchema(fh.mgmtInfo);
    rel->name = name;
    rel->mgmtData = malloc(sizeof(tableInfo));
    memcpy(&rel->mgmtData->totalTuple, bm_ph->data + (int)schemaLength, sizeof(int));
    memcpy(&rel->mgmtData->rid, bm_ph->data + (int)schemaLength + sizeof(int), sizeof(int));
    
    //clean data
    bm_ph = NULL;
    free(bm_ph);
    
//    printf("Open Table Success!");
    
    return RC_OK;
}

RC closeTable (RM_TableData *rel){
    CHECK_POINTER(rel);
    
    BM_PageHandle *bm_ph = MAKE_PAGE_HANDLE();
    
    //update table info
    stateChecker = pinPage(rel->bm, bm_ph, 0);
    CHECK_STATE(stateChecker, -6, "PIN_PAGE_FAILED");
    
    //update totalTuples
    memcpy(bm_ph->data + schemaLength, &rel->mgmtData->totalTuple, sizeof(int));
    
    //update rid
    memcpy(bm_ph->data + schemaLength + sizeof(int), &rel->mgmtData->rid, sizeof(int));
    
    stateChecker = unpinPage(rel->bm, bm_ph);
    CHECK_STATE(stateChecker, -7, "UNPIN_PAGE_FAILED");
    
    stateChecker = markDirty(rel->bm, bm_ph);
    CHECK_STATE(stateChecker, -8, "MARK_DIRTY_FAILED");
    
    //free pointers
    freeSchema(rel->schema);
    
    stateChecker = shutdownBufferPool(rel->bm);                                 //write into file
    CHECK_STATE(stateChecker, -1, "SHUT_DOWN_BUFFER_POOL_FAILED");
    
    //clean data
    rel->mgmtData->rid = 0;
    rel->mgmtData->totalTuple = 0;
    rel->bm->strategy = -1;
    rel->bm->pageFile = NULL;
    rel->bm->numPages = 0;
    rel->name = NULL;
    bm_ph = NULL;
    free(rel->mgmtData);
    free(rel->bm);
    free(bm_ph);
    
    return RC_OK;
}

RC deleteTable (char *name){
    
    //detory pageFile
    stateChecker = destroyPageFile(name);
    CHECK_STATE(stateChecker, -1, "DESTORY_FILE_FAILED");
    
    return RC_OK;
}

int getNumTuples (RM_TableData *rel){

    return rel->mgmtData->totalTuple;
}

/****************** Record Function ******************/

RC insertRecord (RM_TableData *rel, Record *record){
    CHECK_POINTER(rel);
    CHECK_POINTER(record);
    
    int pos;                            //slot position
    BM_PageHandle* bm_ph = malloc(sizeof(BM_PageHandle));

    //check slot position
    if(rel->mgmtData->rid < maxPageSlot){                  //slot in first page
        record->id.page = 0;
        record->id.slot = rel->mgmtData->rid;
    }
    else{                                                   //slot in other page
        record->id.page = (int)rel->mgmtData->rid / maxPageSlot;
        record->id.slot = (int)rel->mgmtData->rid % maxPageSlot;
    }
    
    if(Flag[rel->mgmtData->rid] == 0){                              //chcek if the slot avai
        pos = headerLength + record->id.slot * slotSize;
        
        //read page into buffer
        stateChecker = pinPage(rel->bm, bm_ph, record->id.page);
        CHECK_STATE(stateChecker, -6, "PIN_PAGE_FAILED");
        
        //copy data into buffer
        memcpy(bm_ph->data + pos, record->data, slotSize);
        
        stateChecker =  markDirty(rel->bm, bm_ph);
        CHECK_STATE(stateChecker, -8, "MARK_DIRTY_FAILED");
        
        stateChecker = unpinPage(rel->bm, bm_ph);
        CHECK_STATE(stateChecker, -7, "UNPIN_PAGE_FAILED");
        
        Flag[rel->mgmtData->rid] = 1;               //set Flag as in use
        
        rel->mgmtData->totalTuple ++;               //total +1
        rel->mgmtData->rid ++;                      //next avai slot +1
    }
    else{
        THROW(RC_DUPLICATE_SLOT, "SLOT_ALREADY_TAKEN");
    }
    
    bm_ph = NULL;
    free(bm_ph);

    return RC_OK;
}

RC deleteRecord (RM_TableData *rel, RID id){
    //mark this record as deleted, we will use destoryRecord Function to hard delete the record
    CHECK_POINTER(rel);
    
    //find the rid for this record
    int rid = id.page * (int)maxPageSlot + id.slot;
    
    //check if the slot has data
    if(Flag[rid] == 1){
        
        Flag[rel->mgmtData->rid] = -1;                  //mark as deleted
        rel->mgmtData->totalTuple--;
    }

    return RC_OK;
}

RC updateRecord (RM_TableData *rel, Record *record){
    CHECK_POINTER(rel);
    CHECK_POINTER(record);
    
    BM_PageHandle *bm_ph = MAKE_PAGE_HANDLE();
    int pos;
    int rid = record->id.page * (int)maxPageSlot + record->id.slot;

    if (Flag[rid] == 1) {
        
        //read page into buffer
        stateChecker = pinPage(rel->bm, bm_ph, record->id.page);
        CHECK_STATE(stateChecker, -6, "PIN_PAGE_FAILED");
        
        pos = headerLength + record->id.slot * slotSize;
        
        //update buffer
        memcpy(bm_ph->data + pos, record->data, slotSize);
        
        stateChecker = markDirty(rel->bm, bm_ph);
        CHECK_STATE(stateChecker, -8, "MARK_DIRTY_FAILED");
        
        stateChecker = unpinPage(rel->bm, bm_ph);
        CHECK_STATE(stateChecker, -7, "UNPIN_PAGE_FAILED");
    }else{
        THROW(RC_DUPLICATE_SLOT, "NULL_SLOT OR SLOT_DELETED");
    }
    
    bm_ph = NULL;
    free(bm_ph);
    
    return RC_OK;
}

RC getRecord (RM_TableData *rel, RID id, Record *record){
    CHECK_POINTER(rel);
    CHECK_POINTER(record);
    
    BM_PageHandle *bm_ph = MAKE_PAGE_HANDLE();
    int rid = record->id.page * (int)maxPageSlot + record->id.slot;

    if (Flag[rid] == 1) {                        //check if the record is alive
        //find pos
        int pos = headerLength + id.slot * slotSize;
        
        //read page into buffer
        stateChecker = pinPage(rel->bm, bm_ph, id.page);
        CHECK_STATE(stateChecker, -6, "PIN_PAGE_FAILED");
        
        record->data = (char*)calloc(slotSize, sizeof(char));
        
        //fill record
        record->id = id;
        memcpy(record->data, bm_ph->data + pos, slotSize);
        
        stateChecker = unpinPage(rel->bm, bm_ph);
        CHECK_STATE(stateChecker, -7, "UNPIN_PAGE_FAILED");
    }

    bm_ph = NULL;
    free(bm_ph);

    return RC_OK;
}

/**************** Scan Function *****************/

RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){
    CHECK_POINTER(rel);
    CHECK_POINTER(scan);
    CHECK_POINTER(cond);
    
    //init scanHandle
    scan->mgmtData = malloc(sizeof(RM_ScanCounter));
    scan->mgmtData->rid = 0;
    scan->mgmtData->cond = cond;
    scan->mgmtData->ID.page = 0;
    scan->mgmtData->ID.slot = 0;
    scan->mgmtData->currentSlot = 0;
    scan->mgmtData->curretnPage = 0;
    scan->rel = rel;
    
    return RC_OK;
}

RC next(RM_ScanHandle *scan, Record *record){
    CHECK_POINTER(scan);
    CHECK_POINTER(record);
    
    Record *fileRecord = MAKE_RECORD();
    Value *result = MAKE_VALUE_PTR;
    BM_PageHandle *bm_ph = MAKE_PAGE_HANDLE();
    
    //get maxpage numbers
    int maxPage = scan->mgmtData->rid / (int)maxPageSlot;

    //scan pages
    for (scan->mgmtData->curretnPage; scan->mgmtData->curretnPage <= maxPage; scan->mgmtData->curretnPage++) {
        stateChecker = pinPage(scan->rel->bm, bm_ph, scan->mgmtData->curretnPage);
        CHECK_STATE(stateChecker, -6, "PIN_PAGE_FAILED");
        
            //scan slots
            for (scan->mgmtData->currentSlot; scan->mgmtData->currentSlot < (int)maxPageSlot; scan->mgmtData->currentSlot++) {
                scan->mgmtData->ID.page = scan->mgmtData->curretnPage;
                scan->mgmtData->ID.slot = scan->mgmtData->currentSlot;
                
                stateChecker = getRecord(scan->rel, scan->mgmtData->ID, fileRecord);
                CHECK_STATE(stateChecker, -10, "GET_RECORD_FAILED");
                
                evalExpr(fileRecord, scan->rel->schema, scan->mgmtData->cond, &result);
                if(result->v.boolV){
                    
                    //filling result
                    record->id.page = fileRecord->id.page;
                    record->id.slot = fileRecord->id.slot;
                    record->data = fileRecord->data;
                    
                    stateChecker = unpinPage(scan->rel->bm, bm_ph);
                    CHECK_STATE(stateChecker, -7, "UNPIN_PAGE_FAILED");
                    
                    scan->mgmtData->rid++;
                    scan->mgmtData->currentSlot++;
                    
                    //clean data
                    result = NULL;
                    fileRecord = NULL;
                    free(fileRecord);
                    free(result);
                    
                    return RC_OK;
                }
                scan->mgmtData->rid++;
                if(scan->mgmtData->rid == scan->rel->mgmtData->rid){                        //all slot has been scaned
                    
                    stateChecker = unpinPage(scan->rel->bm, bm_ph);
                    CHECK_STATE(stateChecker, -7, "UNPIN_PAGE_FAILED");
                    
                    //clean data
                    result = NULL;
                    fileRecord = NULL;
                    free(result);
                    free(fileRecord);
                    return RC_RM_NO_MORE_TUPLES;
                }
            }
        stateChecker = unpinPage(scan->rel->bm, bm_ph);
        CHECK_STATE(stateChecker, -7, "UNPIN_PAGE_FAILED");
        
        }

    return RC_RM_NO_MORE_TUPLES;
}

RC closeScan (RM_ScanHandle *scan){
    
    //clean scan
    scan->mgmtData->cond = NULL;
    scan->mgmtData->currentSlot = 0;
    scan->mgmtData->curretnPage = 0;
    scan->mgmtData->ID.page = 0;
    scan->mgmtData->ID.slot = 0;
    scan->mgmtData->rid = 0;
    scan->rel = NULL;
    free(scan->rel);
    free(scan->mgmtData);
    
    return RC_OK;
}

/*************** Schema Funciton ****************/

int getRecordSize (Schema *schema){
    CHECK_POINTER(schema);
    
    int result = 0;
    int i = 0;
    do{
        result = result + getAttrSize(schema, i);
        i++;
    }while(i < schema->numAttr);
    
    return result;
    
}

Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){

    Schema *nSchema = MAKE_SCHEMA();
    nSchema->numAttr = numAttr;
    nSchema->attrNames = attrNames;
    nSchema->dataTypes = dataTypes;
    nSchema->typeLength = typeLength;
    nSchema->keySize = keySize;
    nSchema->keyAttrs = keys;
    return nSchema;
}

RC freeSchema (Schema *schema){
    CHECK_POINTER(schema);
    
    schema->keyAttrs = NULL;
    schema->typeLength = NULL;
    schema->dataTypes = NULL;
    schema->attrNames = NULL;
    schema->keySize = 0;
    schema->numAttr = 0;
    free(schema->keyAttrs);
    free(schema->typeLength);
    free(schema->dataTypes);
    for (int i = 0; i < schema->numAttr; i++){
        free(schema->attrNames[i]);
    }
    free(schema->attrNames);
    
    schema = NULL;
    free(schema);
    return RC_OK;
}

/****************** Record & Attribute Functon *********************/

RC createRecord (Record **record, Schema *schema){
    CHECK_POINTER(record);
    CHECK_POINTER(schema);
    
    *record = MAKE_RECORD();
    (*record)->data = (char*)calloc(slotSize, sizeof(char));
    
    return RC_OK;
}

RC freeRecord (Record *record){
    CHECK_POINTER(record);
    
    record->data = NULL;
    record->id.page = 0;
    record->id.slot = 0;
    free(record->data);
    free(record);
    
    return RC_OK;
}



RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){
    CHECK_POINTER(record);
    CHECK_POINTER(schema);
    CHECK_POINTER(value);
    
    int offset = 0;
    int i;
    for (i = 0; i < attrNum; i++) {
        offset = offset + getAttrSize(schema, i);
    }
    
    int size = getAttrSize(schema, attrNum);
    
    *value = malloc(sizeof(Value));
    (*value)->dt = schema->dataTypes[attrNum];
    
    if ((*value)->dt == DT_STRING) {
        (*value)->v.stringV = (char *) malloc(size + 1);
        memcpy((*value)->v.stringV, record->data + offset, size);
        (*value)->v.stringV[size] = '\0';
    } else {
        memcpy(&((*value)->v), record->data + offset, size);
    }

    return RC_OK;
}

RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
    CHECK_POINTER(record);
    CHECK_POINTER(schema);
    CHECK_POINTER(value);
    
    int offset = 0;
    int i;
    for (i = 0; i < attrNum; i++) {
        offset = offset + getAttrSize(schema, i);
    }
    
    int size = getAttrSize(schema, i);
    
    if (value->dt == DT_STRING) {
        memcpy(record->data + offset, value->v.stringV, size);
    } else {
        memcpy(record->data + offset, &(value->v), size);
    }
    
    return RC_OK;
}
