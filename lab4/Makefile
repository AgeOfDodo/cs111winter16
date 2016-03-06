# Makefile for lab4

# gcc sltest.c -o sltest -pthread 


CC = gcc
CFLAGS = -Wformat=0
LDLIBS = -pthread

all: srt

srt_objects = addtest sltest
srt: $(srt_objects)
	$(CC) $(CFLAGS) -o $@ $(srt_objects) $(LDLIBS)

# header files
sltest: .h
	$(CC) $(CFLAGS) -o sltest.c sltest $(LDLIBS)
# check: baseline.ppm $(tests)


dist: lab4-SunnieKim.tgz
sources = Makefile addtest.c sltest.c README.md 

lab4-SunnieKim.tgz: $(sources)
	tar cf - --transform='s|^|srt/|' $(sources) | gzip -9 >$@

clean:
	rm -f addtest sltest lab4-SunnieKim.tgz
