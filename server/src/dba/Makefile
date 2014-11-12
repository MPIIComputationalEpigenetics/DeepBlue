include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libdba.a
OBJS    = changes.o clone.o config.o collections.o data.o dba.o column_types.o experiments.o key_mapper.o helpers.o queries.o remove.o controlled_vocabulary.o full_text.o insert.o retrieve.o info.o genomes.o list.o metafield.o users.o

all : $(OBJLIBS)

../libdba.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libdba.a $?
	$(RANLIB) ../libdba.a

clean :
	$(ECHO) cleaning up in dba/
	-$(RM) -f $(OBJS)

force_look :
	true
