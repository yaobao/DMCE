# This is an Makefile for the programs PARMCE
Objects = main.o  

EXECS = parmce

default: parmce

parmce: $(Objects)
	mpic++ -o parmce $(Objects) -O3

clean:
	rm -rf *.o $(EXECS)

.cpp.o:
	mpic++ -c $<

.SUFFIXES: .cpp
