include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libmdbq.a
OBJS		= client.o hub.o

all : $(OBJLIBS)

../libmdbq.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libmdbq.a $?
	$(RANLIB) ../libmdbq.a

clean :
	$(ECHO) cleaning up in mdbq/
	-$(RM) -f $(OBJS)

force_look :
	true
