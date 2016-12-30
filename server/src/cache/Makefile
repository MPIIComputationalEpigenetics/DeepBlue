include ../Makefile.inc

ROOT		= $(shell pwd)/../..
CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libcache.a
OBJS    = column_dataset_cache.o queries_cache.o

all : $(OBJLIBS)

../libcache.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libcache.a $?
	$(RANLIB) ../libcache.a

clean :
	$(ECHO) cleaning up in cache/
	-$(RM) -f $(OBJS)

force_look :
	true
