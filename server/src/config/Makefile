include ../Makefile.inc

ROOT		= $(shell pwd)/../..
CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libconfig.a
OBJS		= config.o

all : $(OBJLIBS)

../libconfig.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libconfig.a $?
	$(RANLIB) ../libconfig.a

clean :
	$(ECHO) cleaning up in config/
	-$(RM) -f $(OBJS)

force_look :
	true
