include ../Makefile.inc

ROOT		= $(shell pwd)/../..
CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libprocessing.a
OBJS	= enrichment_result.o binning.o calculate_enrichment.o lola.o count_regions.o coverage.o distinct.o get_experiments_by_query.o get_regions.o processing.o running_cache.o score_matrix.o signature.o

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
