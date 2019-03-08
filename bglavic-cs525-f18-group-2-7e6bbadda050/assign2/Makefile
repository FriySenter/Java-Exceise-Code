
all: test_assign2

DEPS=test_assign2_1.o storage_mgr.o storage_mgr_stat.o dberror.o buffer_mgr.o

test_assign2: test_assign2_1.o dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o 
	gcc -g test_assign2_1.o dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o -o test_assign2

test_assign2_1.o: test_helper.h test_assign2_1.c 
	gcc -g -c test_assign2_1.c

storage_mgr.o: storage_mgr.h storage_mgr.c 
	gcc -g -c storage_mgr.c


buffer_mgr.o: buffer_mgr.h buffer_mgr.c 
	gcc -g -c buffer_mgr.c

storage_mgr_stat.o: buffer_mgr_stat.h  buffer_mgr_stat.c 
	gcc -g -c buffer_mgr_stat.c

dberror.o: dberror.h dberror.c 
	gcc -g -c dberror.c

clean:
	rm -f $(DEPS) test_assign2
