include ../Makefile.inc

ROOT		= $(shell pwd)/../..
CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libsignature.a
OBJS    = signature.o

all : $(OBJLIBS)

../libsignature.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libsignature.a $?
	$(RANLIB) ../libsignature.a

clean :
	$(ECHO) cleaning up in signature/
	-$(RM) -f $(OBJS)

force_look :
	true
