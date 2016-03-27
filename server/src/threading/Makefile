include ../Makefile.inc

ROOT		= $(shell pwd)/../..
CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libthreading.a
OBJS    = semaphore.o

all : $(OBJLIBS)

../libthreading.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libthreading.a $?
	$(RANLIB) ../libthreading.a

clean :
	$(ECHO) cleaning up in threading/
	-$(RM) -f $(OBJS)

force_look :
	true
