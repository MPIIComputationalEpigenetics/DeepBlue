include ../Makefile.inc

ROOT		= $(shell pwd)/../..
CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libconnection.a
OBJS		= connection.o connection_pool.o connection_pool_host.o

all : $(OBJLIBS)

../libconnection.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libconnection.a $?
	$(RANLIB) ../libconnection.a

clean :
	$(ECHO) cleaning up in connection/
	-$(RM) -f $(OBJS)

force_look :
	true
