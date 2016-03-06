# Makefile for lab4

# gcc sltest.c -o sltest -pthread 


CC = gcc
CFLAGS = -Wformat=0 -pthread

all: srt

srt_objects = addtest sltest SortedList.h
srt: $(srt_objects)
	$(CC) $(CFLAGS) -o $@ $(srt_objects) $(LDLIBS)


sltest: sltest.c SortedList.h
	$(CC) $(CFLAGS) -o $@ sltest.c $(LDLIBS)
# check: baseline.ppm $(tests)


dist: lab4-SunnieKim.tgz
sources = Makefile addtest.c sltest.c README.md 

lab4-SunnieKim.tgz: $(sources)
	tar cf - --transform='s|^|srt/|' $(sources) | gzip -9 >$@

clean:
	rm -f addtest sltest lab4-SunnieKim.tgz
