include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libengine.a
OBJS	= engine.o commands.o

all : $(OBJLIBS)

../libengine.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libengine.a $?
	$(RANLIB) ../libengine.a

clean :
	$(ECHO) cleaning up in engine/
	-$(RM) -f $(OBJS)
	for d in $(DIRS); do (cd $$d; $(MAKE) clean); done

force_look :
	true
