include Makefile.inc

LIB_DIRS	= algorithms cache connection datatypes dba engine external extras httpd lua mdbq parser processing threading signature storage

EXE	= server

COMMANDS = $(wildcard commands/*.cpp)
COMMANDS_OBJS = $(COMMANDS:.cpp=.o)

OBJS	= main.o version.o errors.o
OBJLIBS	= $(COMMANDS_OBJS) libhttpd.a libparser.a libdba.a libengine.a libprocessing.a libextras.a libalgorithms.a libdatatypes.a liblua.a libexternal.a libmdbq.a libstorage.a libconnection.a libthreading.a libcache.a libsignature.a libconfig.a

CXXFLAGS	= $(DEFCXXFLAGS)

all : $(EXE)

$(EXE) : $(OBJS) $(OBJLIBS)
	$(LD) -o $(EXE) $(OBJS) $(OBJLIBS) $(THIRDLIBS) $(LDFLAGS) $(DEFLIBS)

libcommands.a: force_look
	@$(ECHO) looking into commands : $(MAKE) $(MFLAGS)
	cd commands; $(MAKE) $(MFLAGS)

libconnection.a: force_look
	@$(ECHO) looking into connection : $(MAKE) $(MFLAGS)
	cd connection; $(MAKE) $(MFLAGS)

libconfig.a: force_look
	@$(ECHO) looking into config : $(MAKE) $(MFLAGS)
	cd config; $(MAKE) $(MFLAGS)

libalgorithms.a: force_look
	@$(ECHO) looking into algorithms : $(MAKE) $(MFLAGS)
	@cd algorithms; $(MAKE) $(MFLAGS)

libthreading.a: force_look
	@$(ECHO) looking into threading : $(MAKE) $(MFLAGS)
	@cd threading; $(MAKE) $(MFLAGS)

libhttpd.a: force_look
	@$(ECHO) looking into httpd : $(MAKE) $(MFLAGS)
	@cd httpd; $(MAKE) $(MFLAGS)

libengine.a: force_look
	@$(ECHO) looking into engine : $(MAKE) $(MFLAGS)
	@cd engine; $(MAKE) $(MFLAGS)

libprocessing.a: force_look
	@$(ECHO) looking into processing : $(MAKE) $(MFLAGS)
	@cd processing; $(MAKE) $(MFLAGS)

libdatatypes.a: force_look
	@$(ECHO) looking into datatypes : $(MAKE) $(MFLAGS)
	@cd datatypes; $(MAKE) $(MFLAGS)

libdba.a: force_look
	@$(ECHO) looking into dba : $(MAKE) $(MFLAGS)
	@cd dba; $(MAKE) $(MFLAGS)

libparser.a: force_look
	@$(ECHO) looking into parser : $(MAKE) $(MFLAGS)
	@cd parser; $(MAKE) $(MFLAGS)

libextras.a: force_look
	@$(ECHO) looking into extras : $(MAKE) $(MFLAGS)
	@cd extras; $(MAKE) $(MFLAGS)

liblua.a: force_look
	@$(ECHO) looking into lua : $(MAKE) $(MFLAGS)
	@cd lua; $(MAKE) $(MFLAGS)

libexternal.a: force_look
	@$(ECHO) looking into external : $(MAKE) $(MFLAGS)
	@cd external; $(MAKE) $(MFLAGS)

libmdbq.a: force_look
	@$(ECHO) looking into mdbq : $(MAKE) $(MFLAGS)
	@cd mdbq; $(MAKE) $(MFLAGS)

libpool.a: force_look
	@$(ECHO) looking into poll : $(MAKE) $(MFLAGS)
	@cd pool; $(MAKE) $(MFLAGS)

libsignature.a: force_look
	@$(ECHO) looking into signature : $(MAKE) $(MFLAGS)
	@cd signature; $(MAKE) $(MFLAGS)

libstorage.a: force_look
	@$(ECHO) looking into storage : $(MAKE) $(MFLAGS)
	@cd storage; $(MAKE) $(MFLAGS)

libcache.a: force_look
	@$(ECHO) looking into cache : $(MAKE) $(MFLAGS)
	@cd cache; $(MAKE) $(MFLAGS)

clean:
	$(ECHO) cleaning up in .
	-$(RM) -f $(EXE) $(OBJS) $(OBJLIBS)
	-for d in $(LIB_DIRS); do (cd $$d; $(MAKE) clean ); done

force_look :
	true
