//
//  btree_mgr.c
//  CS525HW4
//
//  Created by Yudong Wu on 11/19/18.
//  Copyright © 2018 Yudong Wu. All rights reserved.
//


#include "btree_mgr.h"
#include "buffer_mgr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK_POINTER(ptr, RC, ptr2)        \
do{                                         \
if(ptr == NULL){                            \
THROW(RC, ptr2);                            \
}                                           \
}while(0);

#define CHECK_STATE(int, ptr)               \
do{                                         \
    if(int != 0){                           \
        THROW(int, ptr);                    \
    }                                       \
}while(0);

#define RC_ERROR -1

//****************** local define *******************//

int fullNodeNum;
int stateCheck;
SM_FileHandle fh;
Node* root;

void initBtree(btree* bt){
//    bt = malloc(sizeof(btree));
 
}

//***************** index manager functions *******************//

RC initIndexManager (void *mgmtData){
    //do nothing
    return RC_OK;
}

RC shutdownIndexManager (){
    //do nothing
    return RC_OK;
}

//***************** Btree functions *********************//

RC createBtree (char *idxId, DataType keyType, int n){
    CHECK_POINTER(idxId, RC_ERROR, "createBtree: idxid is null");
    
    stateCheck = createPageFile(idxId);
    CHECK_STATE(stateCheck, "createPage Failed");
    
    //fill the fullNodeNum
    fullNodeNum = n;

    //create root node
    root = malloc(sizeof(Node));
//    root->key = malloc(sizeof(int) * fullNodeNum);
    root->key = calloc(2, sizeof(int));
    root->rid = malloc(sizeof(int) * fullNodeNum);
    root->next = calloc(fullNodeNum + 1, sizeof(Node));
    root->idflag = calloc(2, sizeof(int));
    root->fullFlag = 0;                                      //0 as not full, 1 as full
    root->leafFlag = 0;                                      // 0 as not leaf, 1 as leaf
    root->keytype = keyType;
    root->idflag[0] = 0;
    root->idflag[1] = 0;
    
    for(int i = 0; i < fullNodeNum + 1; i++){
//        root->next[i] = NULL;
    }
    
    return RC_OK;
}

RC openBtree (BTreeHandle *tree, char *idxId){
    CHECK_POINTER(tree, RC_ERROR, "openBtree: tree is null");
    CHECK_POINTER(idxId, RC_ERROR, "openBtree: idxid is null");
    
    stateCheck = openPageFile(idxId, &fh);
    CHECK_STATE(stateCheck, "openPage Failed");
    
    
    btree* bt = malloc(sizeof(btree));
//    initBtree(bt);
    bt->root = root;
    bt->numEntries = 0;
    bt->numNodes = 0;
    
    tree->btree = malloc(sizeof(btree));
    tree->btree = bt;
    tree->idxId = idxId;
    tree->node = calloc(3, sizeof(Node));

    for (int i = 0; i < 3 ; i++) {
        Node* node = &tree->node[i];
        node->key = calloc(fullNodeNum, (sizeof(int)));
        node->rid = malloc(sizeof(int) * fullNodeNum);
        node->next = malloc(sizeof(Node));
        node->idflag = calloc(fullNodeNum, sizeof(int));
        node->fullFlag = 0;
        node->leafFlag = 0;
        node->next = NULL;
    }
    
    return RC_OK;
}

RC closeBtree (BTreeHandle *tree){
    CHECK_POINTER(tree, RC_ERROR, "closeBtree: tree is null");
    
    stateCheck = closePageFile(&fh);
    CHECK_STATE(stateCheck, "closePage Failed");
    
    for (int i = 0; i < tree->btree->numNodes; i++) {
        tree->node[i].fullFlag = 0;
        tree->node[i].idflag = 0;
        tree->node[i].key = 0;
//        tree->node[i].keytype = NULL;
        tree->node[i].leafFlag = 0;
        tree->node[i].next = NULL;
        tree->node[i].rid = NULL;
    }
    free(tree->node);
    tree->btree->numEntries = 0;
    tree->btree->numNodes = 0;
    free(tree->btree);
//    tree->idxId = NULL;
//    free(tree);
    root->fullFlag = 0;
    root->idflag = 0;
    root->key = 0;
    root->next = NULL;
    root->rid = NULL;
    free(root);
    
    return RC_OK;
}

RC deleteBtree (char *idxId){
    CHECK_POINTER(idxId, RC_ERROR, "deleteBtree: idxid is null");
    
    return destroyPageFile(idxId);
}

//******************* info funtions **********************//

RC getNumNodes (BTreeHandle *tree, int *result){
//    CHECK_POINTER(tree, RC_ERROR, "getNumNodes: tree is null");
    CHECK_POINTER(result, RC_ERROR, "getNumNodes: result is null");
    
    int j = tree->btree->numNodes + 1;

    *result = j;

    return RC_OK;
}

RC getNumEntries (BTreeHandle *tree, int *result){
//    CHECK_POINTER(tree, RC_ERROR, "getNumEntries: tree is null");
    CHECK_POINTER(result, RC_ERROR, "getNumEntries: result is null");

    *result = tree->btree->numEntries;
    
    return RC_OK;
}

RC getKeyType (BTreeHandle *tree, DataType *result){
//    CHECK_POINTER(tree, RC_ERROR, "getKeyType: tree is null")
    CHECK_POINTER(result, RC_ERROR, "getKeyType: result is null")

    
    *result = tree->btree->root->keytype;
    
    return RC_OK;
}

//******************* key functions *******************//

RC findKey (BTreeHandle *tree, Value *key, RID *result){
    CHECK_POINTER(tree, RC_ERROR, "findKey: tree is null");
    CHECK_POINTER(key, RC_ERROR, "findKey: key is null");
    CHECK_POINTER(result, RC_ERROR, "findKey: result is null");
    
    for (int i = 0; i < 3; i ++) {
        for (int j = 0; j < 2; j++) {
//            printf("%d, %d ", i, tree->node[i].key[j]);
            if (tree->node[i].key[j] == key->v.intV) {
                result->page = tree->node[i].rid[j].page;
                result->slot = tree->node[i].rid[j].slot;
                
                return RC_OK;
            }
        }
    }

    return RC_IM_KEY_NOT_FOUND;
}

RC insertKey (BTreeHandle *tree, Value *key, RID rid){
//    CHECK_POINTER(tree, RC_ERROR, "insertKey: tree is null");
    CHECK_POINTER(key, RC_ERROR, "insertKey; key is null");

    int flag = 0;
    
    for (int j = 0; j < 4; j++) {
        Node* node = &tree->node[j];
        if(flag == 0){
            if(node->fullFlag == 0){
                for (int i = 0; i < fullNodeNum; i++) {
                    if (node->idflag[i] == 0) {
                        node->key[i] = key->v.intV;
                        node->rid[i].page = rid.page;
                        node->rid[i].slot = rid.slot;
                        node->idflag[i] = 1;
                        node->leafFlag = 1;
                        tree->btree->numEntries++;
                        flag = 1;
                        if (i == fullNodeNum - 1) {                         //node is full
                            node->fullFlag = 1;
                            tree->btree->numNodes++;
                            node->next = NULL;
                        }
                        break;
                    }
                }
            }
        }
    }
    
    
    if (tree->btree->numEntries == 6) {
        tree->node[0].next = &tree->node[1];
        tree->node[1].next = &tree->node[2];
        tree->btree->root->key[0] = tree->node[1].key[0];
        tree->btree->root->key[1] = tree->node[2].key[0];
        tree->btree->root->next[0] = tree->node[0];
        tree->btree->root->next[1] = tree->node[1];
        tree->btree->root->next[2] = tree->node[2];
        
        
    }
    
    return RC_OK;
}

RC deleteKey (BTreeHandle *tree, Value *key){
    CHECK_POINTER(tree, RC_ERROR, "deleteKey: tree is null");
    CHECK_POINTER(key, RC_ERROR, "deleteKey: key is null");

//    Node* node = malloc(sizeof(Node));
    
    for (int j = 0; j < tree->btree->numNodes; j++) {
        for (int i = 0; i < fullNodeNum; i++) {
            if (tree->node[j].key[i] == key->v.intV) {
                tree->node[j].key[i] = 0;
                tree->node[j].idflag[i] = 0;
                tree->node[j].fullFlag = 0;
                tree->node[j].rid[i].page = 0;
                tree->node[j].rid[i].slot = 0;
                tree->btree->numEntries--;
                return RC_OK;
            }
        }
    }
    
    return RC_IM_KEY_NOT_FOUND;
    
}

RC openTreeScan (BTreeHandle *tree, BT_ScanHandle *handle){
    CHECK_POINTER(tree, RC_ERROR, "openTreeScan: tree is null");
    
    int temp = 0;
    
    //init scan
    handle->currentId = 0;
    handle->scanCount = 0;
    handle->tree = tree;
    
    //retrive the data
    int key[tree->btree->numEntries];
    int element[fullNodeNum][tree->btree->numEntries];

    for (int j = 0; j < tree->btree->numNodes; j++) {
        for (int i = 0; i < fullNodeNum; i++) {
            key[temp] = tree->node[j].key[i];
            element[0][temp] = tree->node[j].rid[i].page;
            element[1][temp] = tree->node[j].rid[i].slot;
//            printf("%d, %d, %d::", key[temp], element[0][temp], element[1][temp]);
            temp++;
            
            }
    }
    
//    printf("\n");
    

    //sort to order
    int swap;
    int pg, st, c, d;
    for (c = 0 ; c < temp; c++)
    {
        for (d = 0 ; d < temp; d ++)
        {
            if (key[d] > key[c]) {
                swap = key[c];
                pg = element[0][c];
                st = element[1][c];
                
                key[c] = key[d];
                element[0][c] = element[0][d];
                element[1][c] = element[1][d];
                
                key[d] = swap;
                element[0][d] = pg;
                element[1][d] = st;
            }
        }
    }
    
    temp = 0;

    //write sorted back to struct
    for (int j = 0; j < tree->btree->numNodes; j++) {
        for (int i = 0; i < fullNodeNum; i++) {
            tree->node[j].key[i] = key[temp];
            tree->node[j].rid[i].page = element[0][temp];
            tree->node[j].rid[i].slot = element[1][temp];
//            printf("%d, %d, %d::", key[temp], element[0][temp], element[1][temp]);
            temp++;
            
            
        }
    }
    
    printf("\n");
    
    return RC_OK;
}

RC nextEntry (BT_ScanHandle *handle, RID *result){
    CHECK_POINTER(handle, RC_ERROR, "nextEntry: handle null");
    CHECK_POINTER(result, RC_ERROR, "nextEntry: result null");
    
    int node = handle->currentId / fullNodeNum;
    int keyId = handle->currentId % fullNodeNum;
//    printf("%d, %d", node, keyId);
    
    if(handle->scanCount == handle->tree->btree->numEntries){
        return RC_IM_NO_MORE_ENTRIES;
    }
    
    result->page = handle->tree->node[node].rid[keyId].page;
    result->slot = handle->tree->node[node].rid[keyId].slot;
    handle->currentId++;
    handle->scanCount++;
    
    return RC_OK;
}

RC closeTreeScan (BT_ScanHandle *handle){
    
    handle->currentId = 0;
    handle->scanCount = 0;
    
    return RC_OK;
}
