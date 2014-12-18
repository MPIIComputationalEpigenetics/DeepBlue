include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libextras.a
OBJS		= utils.o xmlrpc.o serialize.o stringbuilder.o compress.o

all : $(OBJLIBS)

../libextras.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libextras.a $?
	$(RANLIB) ../libextras.a

clean :
	$(ECHO) cleaning up in extras/
	-$(RM) -f $(OBJS)

force_look :
	true
