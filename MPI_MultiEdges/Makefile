# This is an Makefile for the programs PARMCE
Objects = main.o  

EXECS = parmce

default: parmce

parmce: $(Objects)
ifeq ($(DEBUG), 1)
	mpic++ -o parmce $(Objects) -g -pg -DDEBUG
else
	mpic++ -o parmce $(Objects) -g -O3 -DNDEBUG
endif

clean:
	rm -rf *.o $(EXECS)

.cpp.o:
	mpic++ -c $<

.SUFFIXES: .cpp
