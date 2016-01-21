include ../Makefile.inc

ROOT		= $(shell pwd)/../..
CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libexternal.a
OBJS		= geo.o url_loader.o

all : $(OBJLIBS)

../libexternal.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libexternal.a $?
	$(RANLIB) ../libexternal.a

clean :
	$(ECHO) cleaning up in external/
	-$(RM) -f $(OBJS)

force_look :
	true
