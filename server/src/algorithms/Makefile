include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libalgorithms.a
OBJS    = accumulator.o intersection.o merge.o levenshtein.o aggregate.o patterns.o

all : $(OBJLIBS)

../libalgorithms.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libalgorithms.a $?
	$(RANLIB) ../libalgorithms.a

clean :
	$(ECHO) cleaning up in algorithms/
	-$(RM) -f $(OBJS)

force_look :
	true
