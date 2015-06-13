include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libentities.a
OBJS    = users.o

all : $(OBJLIBS)

../libentities.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libentities.a $?
	$(RANLIB) ../libentities.a

clean :
	$(ECHO) cleaning up in httpd/
	-$(RM) -f $(OBJS)

force_look :
	true
