include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libprocessing.a
OBJS	= count_regions.o get_experiments_by_query.o get_regions.o processing.o score_matrix.o

all : $(OBJLIBS)

../libprocessing.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libprocessing.a $?
	$(RANLIB) ../libprocessing.a

clean :
	$(ECHO) cleaning up in processing/
	-$(RM) -f $(OBJS)
	for d in $(DIRS); do (cd $$d; $(MAKE) clean); done

force_look :
	true
