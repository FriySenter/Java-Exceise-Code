Group 2

Yudong WU     A20405374
Bin Sun       A20376746
Hailong Mao   A20379619


operating steps(in Linux system, suppose gcc is installed)
  1. Download the source code 
  2. Open the Linux terminal, use command "cd" to access this folder.
  3. Input command "make", press Enter to run the
  4. Input command "./test_assign1", press Enter to test.

CS525
Assignment 1

README

For this assign, all of the functions were wrote in ‘storage_mgr.c’

Task Assignment:
File operation functions, code reorganization, README file



/******************************************************
*				function detail				   *
******************************************************/

void initStorageManager will be the interface for further use. For this assignment, we could just do nothing.

formatPage function will format a blank page, the size will be a page size.

createPageFile function will create a page file with a blank page.

openPageFile function will open a page file and allow the user to read and write the file. Then write the info form the page file to the fhandles.

closePageFile function will simply close the page file, and set the mgmtInfo to null.

destroyPageFile function will check If the file exist, then destroy the page file.

readBlock function will read the data from the block.

getBlockPos function will return the current location.

readFirstBlock function will simply return the data from the first block.

readPreviousBlock function will check if the current position is the first block, then return of the previous block.

readCurrentBlock function will simply return the data form current block.

readNextBlock function will check if the current position is the last block of this page file, then return the data from next block.

readLastBlock function will return the data from the last block.

writeBlock function will write the data to the block where the pointer is.

writeCurrentBlock function will write the data to the current block.

appendEmptyBlock function using the format page function to append an empty page.

ensureCapacity function will calculate how many pages are required to add, then add them.



