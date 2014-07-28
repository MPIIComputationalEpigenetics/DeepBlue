UNAME_S := $(shell uname -s)
UNAME_N := $(shell uname -n)

host =

ifeq ($(UNAME_S),Linux)
	host = linux
	ifeq ($(UNAME_N),infao3702)
		host = production
	endif
	ifeq ($(UNAME_N),infao6940)
		host = production
	endif
endif
ifeq ($(UNAME_S),Darwin)
	host = macosx
endif

# Servers
ifeq ($(host),production)
 CXX = g++
 LD  = g++
 LDFLAGS     = -static -pthread
 MONGOLIBS = -L/opt/mongodb/driver/lib -lmongoclient
 BOOST_DIR = /opt/mongodb/boost/lib
 DEFCXXFLAGS = -Wall -O3 -Ofast -I/opt/mongodb/boost/include/ -I/opt/mongodb/driver/include/ -I/opt/mongodb/DeepBlue/server/third_party/cppformat/
 BOOSTLIBS = $(BOOST_DIR)/libboost_filesystem.a $(BOOST_DIR)/libboost_thread.a $(BOOST_DIR)/libboost_system.a $(BOOST_DIR)/libboost_regex.a $(BOOST_DIR)/libboost_log.a  $(BOOST_DIR)/libboost_timer.a  $(BOOST_DIR)/libboost_chrono.a $(BOOST_DIR)/libboost_program_options.a -lrt
 THIRDLIBS = ../third_party/libexpat_linux.a

endif

# -O3 -Ofast
# Felipe Albrecht's mac
ifeq ($(host),macosx)
 CXX = clang++
 DEFCXXFLAGS = -g   -Werror -Wall -Wextra  -Weffc++ -Wno-unused-parameter -Wno-ignored-qualifiers -DPROFINY_CALL_GRAPH_PROFILER -I/opt/boost/include/ -I/Users/albrecht/mpi/DeepBlue/server/third_party/drivers/macosx/include -I/Users/albrecht/mpi/DeepBlue/server/third_party/cppformat/ -I/Users/albrecht/mpi/DeepBlue/server/third_party/minilzo/
 BOOST_DIR = /opt/boost/lib
 LD	= clang++
 LDFLAGS = -g
 MONGOLIBS =  -L/Users/albrecht/mpi/DeepBlue/server/third_party/drivers/macosx/lib -lmongoclient
 BOOSTLIBS = $(BOOST_DIR)/libboost_filesystem.a $(BOOST_DIR)/libboost_thread.a $(BOOST_DIR)/libboost_system.a $(BOOST_DIR)/libboost_regex.a $(BOOST_DIR)/libboost_log.a  $(BOOST_DIR)/libboost_timer.a  $(BOOST_DIR)/libboost_chrono.a $(BOOST_DIR)/libboost_program_options.a
 THIRDLIBS = ../third_party/libexpat_macosx.a
endif

CPPFORMAT = ../third_party/cppformat/format.cc ../third_party/minilzo/minilzo.c
DEFLIBS = $(MONGOLIBS) $(BOOSTLIBS) $(CPPFORMAT)

AR	= ar
ARFLAGS	=
RANLIB	= ranlib

RM	= rm
ECHO	= echo

SHELL	= /bin/sh
