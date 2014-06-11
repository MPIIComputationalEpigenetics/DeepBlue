include Makefile.inc

LIB_DIRS	= httpd engine dba parser extras algorithms

EXE	= server

COMMANDS = $(wildcard commands/*.cpp)
COMMANDS_OBJS = $(COMMANDS:.cpp=.o)

OBJS	= main.o version.o regions.o
OBJLIBS	= $(COMMANDS_OBJS) libhttpd.a libdba.a libparser.a libengine.a libextras.a libalgorithms.a

CXXFLAGS	= $(DEFCXXFLAGS)

all : $(EXE)

$(EXE) : $(OBJS) $(OBJLIBS)
	$(LD) -o $(EXE) $(OBJS) $(OBJLIBS) $(THIRDLIBS) $(LDFLAGS) $(DEFLIBS)

libcommands.a: force_look
	@$(ECHO) looking into commands : $(MAKE) $(MFLAGS)
	cd commands; $(MAKE) $(MFLAGS)

libalgorithms.a: force_look
	@$(ECHO) looking into algorithms : $(MAKE) $(MFLAGS)
	@cd algorithms; $(MAKE) $(MFLAGS)

libhttpd.a: force_look
	@$(ECHO) looking into httpd : $(MAKE) $(MFLAGS)
	@cd httpd; $(MAKE) $(MFLAGS)

libengine.a: force_look
	@$(ECHO) looking into engine : $(MAKE) $(MFLAGS)
	@cd engine; $(MAKE) $(MFLAGS)

libdba.a: force_look
	@$(ECHO) looking into dba : $(MAKE) $(MFLAGS)
	@cd dba; $(MAKE) $(MFLAGS)

libparser.a: force_look
	@$(ECHO) looking into parser : $(MAKE) $(MFLAGS)
	@cd parser; $(MAKE) $(MFLAGS)

libextras.a: force_look
	@$(ECHO) looking into extras : $(MAKE) $(MFLAGS)
	@cd extras; $(MAKE) $(MFLAGS)

clean :
	$(ECHO) cleaning up in .
	-$(RM) -f $(EXE) $(OBJS) $(OBJLIBS)
	-for d in $(LIB_DIRS); do (cd $$d; $(MAKE) clean ); done

force_look :
	true
