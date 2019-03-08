Group 2

Yudong Wu     A20405374
Bin Sun       A20376746
Hailong Mao   A20379619


1 operating steps(in Linux system, suppose gcc is installed)
  1. Download the source code 
  2. Open the Linux terminal, use command "cd" to access this folder.
  3. type command "make", press Enter to run the
  4. type command "./test_assign3_1", press Enter to test.


2 the file list

                Makefile
                README.txt
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
		test_assign3_1.c
		test_helper.h



3 functions be implement

//These 2 functions are defined in local and used for util.
getAttrSize: get Attribute size 
convertSchema: deserialize the schema

//following are the functions covered by Record_mgr.h
initRecordManager: inital the record manager
shutdownRecordManager: shut down the record manager
createTable: create a table
openTable: open a table
closeTable: close a table
deleteTable: delete a table
getNumTuples:returns the number of tuples in the table


insertRecord:insert a record with a certain RID
deleteRecord:delete a record with a certain RID
updateRecord:update an existing record with new values
getRecord: get a record with a certain RID

startScan:initializes the RM_ScanHandle data structure passed as an argument to startScan
next: return the next tuple that fulfills the scan condition
closeScan: close the scan

getRecordSize:Get the size of record
createSchema: create the schema
freeSchema:free the schema

createRecord: create the record
freeRecord: free the record 
getAttr: get the value of a record
setAttr: set the value of a record

Data Structure we use:

We use talbeInfo to store the Schema and Total number of tuples and next available slot number.

The tableInfo struct is showed blow:

typedef struct tableInfo{
    char* charSchema;               //store scheme of table
    int totalTuple;                //total tuple number
    int rid;                		//next avai RID
}tableInfo;

For deleting the recoreds, we will mark this record as deleted which will be like the actual Database we use. In case we want to recover these deleted records in future.

// Bookkeeping for scans

typedef struct RM_ScanCounter{
    int rid;                    //slot nubmer;
    int currentSlot;		
    int curretnPage;
    Expr* cond;
    RID ID;
}RM_ScanCounter;


typedef struct RM_ScanHandle
{
    RM_TableData *rel;
    RM_ScanCounter *mgmtData;
} RM_ScanHandle;

typedef struct RM_TableData{
    char *name;
    Schema *schema;
    tableInfo *mgmtData;                //pointer to tableInfo
    BM_BufferPool *bm;                  //pointer to BufferPool
} RM_TableData;

