include ../Makefile.inc

ROOT		= $(shell pwd)/../..
CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libalgorithms.a
OBJS    = count_go_terms.o accumulator.o aggregate.o extend.o flank.o intersection.o merge.o levenshtein.o patterns.o

all : $(OBJLIBS)

../libalgorithms.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libalgorithms.a $?
	$(RANLIB) ../libalgorithms.a

clean :
	$(ECHO) cleaning up in algorithms/
	-$(RM) -f $(OBJS)

force_look :
	true
