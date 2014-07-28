include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libparser.a
OBJS	= parser_factory.o genome_data.o wig.o wig_parser.o fasta_parser.o

all : $(OBJLIBS)

../libparser.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libparser.a $?
	$(RANLIB) ../libparser.a

clean :
	$(ECHO) cleaning up in parser/
	-$(RM) -f $(OBJS)

force_look :
	true
