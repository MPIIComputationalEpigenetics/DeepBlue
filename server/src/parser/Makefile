include ../Makefile.inc

ROOT		= $(shell pwd)/../..
CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libparser.a
OBJS	= cufflinks_parser.o salmon_parser.o salmon.o fpkm.o parser_factory.o grape2.o grape2_parser.o genome_data.o gtf.o gtf_parser.o wig.o bedgraph_parser.o wig_parser.o fasta_parser.o

all : $(OBJLIBS)

../libparser.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libparser.a $?
	$(RANLIB) ../libparser.a

clean :
	$(ECHO) cleaning up in parser/
	-$(RM) -f $(OBJS)

force_look :
	true
