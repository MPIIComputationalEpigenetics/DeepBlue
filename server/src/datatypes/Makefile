include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libdatatypes.a
OBJS    = column_types_def.o metadata.o regions.o

all : $(OBJLIBS)

../libdatatypes.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libdatatypes.a $?
	$(RANLIB) ../libdatatypes.a

clean :
	$(ECHO) cleaning up in datatypes/
	-$(RM) -f $(OBJS)

force_look :
	true
