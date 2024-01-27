SRC := dberror.c storage_mgr.c test_assign1_1.c
OBJ := dberror.o storage_mgr.o test_assign1_1.o

storage_mgr_assign1: $(OBJ)
	gcc -o test_assign1_1 $?
%.o: %.c
	gcc -g -c $<
run: storage_mgr_assign1
	./test_assign1_1
clean:
	rm -rf test_assign1_1 *.o