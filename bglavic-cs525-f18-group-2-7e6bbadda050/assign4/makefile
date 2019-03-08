all: test_assign4_1


OBJECTS= test_assign4_1.o test_expr.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr.o buffer_mgr_stat.o btree_mgr.o



test_assign4_1: test_assign4_1.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o btree_mgr.o
	gcc -g test_assign4_1.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o btree_mgr.o -o test_assign4_1


	

test_assign4_1.o: test_assign4_1.c
	gcc -g -c test_assign4_1.c

	
test_expr.o: test_expr.c
	gcc -g -c test_expr.c

record_mgr.o: record_mgr.c
	gcc -g -c record_mgr.c

rm_serializer.o: rm_serializer.c
	gcc -g -c rm_serializer.c

expr.o: expr.c
	gcc -g -c expr.c

storage_mgr.o: storage_mgr.c
	gcc -g -c storage_mgr.c

dberror.o: dberror.c
	gcc -g -c dberror.c

buffer_mgr.o: buffer_mgr.c
	gcc -g -c buffer_mgr.c

buffer_mgr_stat.o: buffer_mgr_stat.c
	gcc -g -c buffer_mgr_stat.c

btree_mgr.o: btree_mgr.c
	gcc -g -c btree_mgr.c

clean:
	rm -f $(OBJECTS) test_assign4_1