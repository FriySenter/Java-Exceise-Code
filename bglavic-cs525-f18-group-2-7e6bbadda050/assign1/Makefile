
CC=gcc
CFLAGS=-Wall -c -o 
DEPS=test_assign1_1.o dberror.o storage_mgr.o

all: 	$(DEPS)
	$(CC) $(DEPS) -o test_assign1

test_assign1_1.o: test_assign1_1.c
	$(CC) $(CFLAGS) test_assign1_1.o test_assign1_1.c 

dberror.o: dberror.h dberror.c
	$(CC) $(CFLAGS) dberror.o dberror.c 

storage_mgr.o: storage_mgr.c
	$(CC) $(CFLAGS) storage_mgr.o storage_mgr.c 

clean:
	rm -f $(DEPS) test_assign1
