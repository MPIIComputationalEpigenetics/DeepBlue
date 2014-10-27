include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../liblua.a
OBJS		= sandbox.o

all : $(OBJLIBS)

../liblua.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../liblua.a $?
	$(RANLIB) ../liblua.a

clean :
	$(ECHO) cleaning up in extras/
	-$(RM) -f $(OBJS)

force_look :
	true
