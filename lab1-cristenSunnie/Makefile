# Makefile. CS111 Lab 1a

CC = gcc
#CFLAGS = 
default: simpsh

simpsh: main.c 
	$(CC) main.c -o $@

dist: lab1-CristenAnderson_SunnieSo.tar.gz

submission_files = README main.c Makefile test.sh
lab1-CristenAnderson_SunnieSo.tar.gz: $(submission_files)
	tar -czf $@ $(submission_files) 

check: 
	./test.sh


clean:
	rm -f *.o *.tar.gz simpsh