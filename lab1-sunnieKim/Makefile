# Makefile. CS111 Lab 1a

CC = gcc
#CFLAGS =
DISTDIR=lab1-SunnieKim
default: simpsh

simpsh: main.c prime.c
	$(CC) main.c -o $@
	$(CC) prime.c -o prime
	
dist: lab1-SunnieKim.tar.gz

submission_files = README main.c Makefile test.sh prime.c lab1cResults.pdf
lab1-SunnieKim.tar.gz: $(submission_files)
	tar -czf $@ --transform='s,^,$(DISTDIR)/,' $(submission_files)  


check: 
	./test.sh


clean:
	rm -f *.o *.tar.gz simpsh