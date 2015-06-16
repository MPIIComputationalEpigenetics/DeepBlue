include ../Makefile.inc

CXXFLAGS	= $(DEFCXXFLAGS) -I..

OBJLIBS	= ../libhttpd.a
OBJS	= connection.o download.o reply.o request_handler.o request_parser.o server.o xmlrpc_request.o xmlrpc_parser.o

all : $(OBJLIBS)

../libhttpd.a : $(OBJS)
	$(AR) $(ARFLAGS) r ../libhttpd.a $?
	$(RANLIB) ../libhttpd.a

clean :
	$(ECHO) cleaning up in httpd/
	-$(RM) -f $(OBJS)

force_look :
	true
