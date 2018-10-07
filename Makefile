CC = gcc
CFLAG = -Wall -O2
CLIBS= -lpthread

OBJECT = main.o threadpool.o

all : threadpool_test

threadpool_test : $(OBJECT)
	$(CC) $(CFLAG) -o threadpool_test $(OBJECT) $(CLIBS)

.c.o : 
	$(CC) $(CFLAG) -c -o $*.o $<

clean:
	rm -f threadpool_test $(OBJECT)

.PHONY : clean



