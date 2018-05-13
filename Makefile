CC = gcc
CFLAG = -Wall -O2
CLIBS= -lpthread

OBJECT = main.o threadpool.o

threadpool_test : $(OBJECT)
	$(CC) $(CFLAG) -o threadpool_test $(OBJECT) $(CLIBS)

main.o : main.c
	$(CC) $(CFLAG) -c main.c

threadpool.o : threadpool.c
	$(CC) $(CFLAG) -c threadpool.c

clean:
	rm -f threadpool_test $(OBJECT)

.PHONY : clean



