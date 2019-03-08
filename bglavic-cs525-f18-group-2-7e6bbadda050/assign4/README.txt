README

Assignment 4
CS525
Group 2

Yudong WU    A20405374
Hailong Mao  A20379619
Bin Sun      A20376746

1 operating steps(in Linux system, suppose gcc is installed)
  1. Download the source code 
  2. Open the Linux terminal, use command "cd" to access this folder.
  3. type command "make", press Enter to run the
  4. type command "./test_assign4_1", press Enter to test.

2 the file list

                Makefile
                README.txt
                btree_mgr.c
                btree_mgr.h
		buffer_mgr.h
                buffer_mgr.c
		buffer_mgr_stat.c
		buffer_mgr_stat.h
		dberror.c
		dberror.h
                record_mgr.cs
                record_mgr.h
		dt.h
                expr.c
                expr.h
                tables.h
		storage_mgr.h
                storage_mgr.c
                test_expr.c
                rm_serializer.c
		test_assign4_1.c
		test_helper.h

3 functions be implement
	// init and shutdown index manager
	RC initIndexManager (void *mgmtData);
	RC shutdownIndexManager ();

	// create, destroy, open, and close an btree index
	RC createBtree (char *idxId, DataType keyType, int n);
	RC openBtree (BTreeHandle **tree, char *idxId);
	RC closeBtree (BTreeHandle *tree);
	RC deleteBtree (char *idxId);

	// access information about a b-tree
	RC getNumNodes (BTreeHandle *tree, int *result);
	RC getNumEntries (BTreeHandle *tree, int *result);
	RC getKeyType (BTreeHandle *tree, DataType *result);

	// index access
	RC findKey (BTreeHandle *tree, Value *key, RID *result);
	RC insertKey (BTreeHandle *tree, Value *key, RID rid);
	RC deleteKey (BTreeHandle *tree, Value *key);
	RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle);
	RC nextEntry (BT_ScanHandle *handle, RID *result);
	RC closeTreeScan (BT_ScanHandle *handle);

4 What we add
	1. add Node Struct:
		typedef struct Node{
    		RID* rid;		//record rid include page & slot
    		int* key;		//record key
    		int* idflag;		//Entries flag
    		bool fullFlag;		//Node full flag
    		bool leafFlag;		//is the node is leaf or not?
    		DataType keytype;	//DataType for this node
    		struct Node* next;	//link to next node
		}Node;

	2. add Btree strcut:
		typedef struct btree{
    		Node* root;		//Btree root
    		int numNodes;		//total number of node
    		int numEntries;		//total number of entries
		}btree;

	3. change BTreeHandle Struct
		typedef struct BTreeHandle {
  			DataType keyType;
  			char *idxId;
  			btree* btree;
  			Node* node;
		} BTreeHandle;

	4. change BY_ScanHandle Struct
		typedef struct BT_ScanHandle {
  		    BTreeHandle *tree;
		    int currentId;		//current scan location
    		    int scanCount;		//total scan location 
		} BT_ScanHandle;



