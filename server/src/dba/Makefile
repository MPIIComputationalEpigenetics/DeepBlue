include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libdba.a
OBJS    = config.o collections.o dba.o  column_types.o key_mapper.o helpers.o queries.o controlled_vocabulary.o full_text.o insert.o retrieve.o info.o genomes.o list.o metafield.o

all : $(OBJLIBS)

../libdba.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libdba.a $?
	$(RANLIB) ../libdba.a

clean :
	$(ECHO) cleaning up in dba/
	-$(RM) -f $(OBJS)

force_look :
	true
