# Makefile for lab4

# gcc sltest.c -o sltest -pthread 


CC = gcc
CFLAGS = -Wformat=0 -pthread
DISTDIR = lab4-SunnieKim
submission_files = Makefile addtest.c sltest.c SortedList.h README.md answers.txt graphs.pdf 

all: sltest addtest

sltest: sltest.c SortedList.h
	$(CC) $(CFLAGS) -o $@ sltest.c $(LDLIBS)

addtest: addtest.c 
	$(CC) $(CFLAGS) -o $@ addtest.c $(LDLIBS)

tarball: lab4-SunnieKim.tgz

lab4-SunnieKim.tgz: $(sources)
	tar -czf $@ --transform='s,^,$(DISTDIR)/,' $(submission_files)  

clean:
	rm -f addtest sltest lab4-SunnieKim.tgz
