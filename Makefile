obj-m+=MAFilter.o 
MAFilter-objs := dataStore.o strNumConv.o movingAverageFilter.o maFilter.o


all:
	# HACK!!!!...  can't figure out how to make Netbeans set PWD correctly
	make -C /lib/modules/$(shell uname -r)/build/ M=/home/pooh/Documents/projects/MAFilter modules
	$(CC) tinker.c -o tinker
	$(CC) -DTEST_COMPILE eyore.c strNumConv.c dataStore.c movingAverageFilter.c testStubs.c -o eyore
	./eyore
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm tinker
