include ../Makefile.inc

ROOT		= $(shell pwd)/../..
CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libstorage.a
OBJS    = storage.o

all : $(OBJLIBS)

../libstorage.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libstorage.a $?
	$(RANLIB) ../libstorage.a

clean :
	$(ECHO) cleaning up in storage/
	-$(RM) -f $(OBJS)

force_look :
	true
