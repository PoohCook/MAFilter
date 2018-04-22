obj-m+=MAFilter.o 
MAFilter-objs := dataStore.o strNumConv.o movingAverageFilter.o maFilter.o


all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) tinker.c -o tinker
	$(CC) -DTEST_COMPILE eyore.c strNumConv.c dataStore.c movingAverageFilter.c testStubs.c -o eyore
	./eyore
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm tinker
