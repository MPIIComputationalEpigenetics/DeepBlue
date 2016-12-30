include ../Makefile.inc

ROOT		= $(shell pwd)/../..
CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libdba.a
OBJS    = annotations.o changes.o clone.o config.o column_types.o collections.o controlled_vocabulary.o data.o datatable.o dba.o  experiments.o exists.o genes.o genomes.o key_mapper.o helpers.o queries.o remove.o full_text.o insert.o retrieve.o sequence_retriever.o info.o genomes.o list.o metafield.o users.o

all : $(OBJLIBS)

../libdba.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libdba.a $?
	$(RANLIB) ../libdba.a

clean :
	$(ECHO) cleaning up in dba/
	-$(RM) -f $(OBJS)

force_look :
	true
