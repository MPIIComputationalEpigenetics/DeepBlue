UNAME_S := $(shell uname -s)
UNAME_N := $(shell uname -n)


ifeq ($(UNAME_S),Linux)
	host = ubuntu
	ifeq ($(UNAME_N),infao3702)
		host = production
	endif
	ifeq ($(UNAME_N),infao6940)
		host = production
	endif
	ifeq ($(UNAME_N),infao3901)
		host = production
	endif
	ifeq ($(UNAME_N),srv-13-41)
		host = new_server
	endif
	ifeq ($(UNAME_N), pih-mpi)
		host = ubuntu
	endif

endif
ifeq ($(UNAME_S),Darwin)
	host = macosx
endif

host = ubuntu

#Root Directory
ROOT=$(shell pwd)/..

host=ubuntu

LUA=$(ROOT)/third_party/luajit-2.0/src
MINILZO=$(ROOT)/third_party/minilzo
STRTK=$(ROOT)/third_party/strtk
CPPFORMAT=$(ROOT)/third_party/cppformat
URDL=$(ROOT)/third_party/urdl-0.1/include
JEMALLOC=$(ROOT)/third_party/jemalloc

GIT_VERSION := $(shell git describe --abbrev=8 --dirty --always --tags)
#GIT_URL := $(shell git remote get-url origin)
GIT_URL = github

ifeq ($(host),ubuntu)
 CXX = g++ -DBOOST_LOG_DYN_LINK -DGIT_VERSION=\"$(GIT_VERSION)\" -DGIT_URL=\"$(GIT_URL)\"
 LD  = g++
 LDFLAGS   = -pthread
 MONGOLIBS = /opt/mongodb/drivers/lib/libmongoclient.a
 MONGO_INCLUDE = /opt/mongodb/drivers/include/
 BOOST_DIR = /opt/boost/lib
 DEFCXXFLAGS = -std=c++11 -Wall -O3 -ffast-math -march=native -Ofast -I$(MONGO_INCLUDE) -I/opt/boost/include/ -I$(MINILZO) -I$(URDL) -I$(CPPFORMAT) -I$(STRTK) -I$(LUA) -I${JEMALLOC}/include  -DURDL_HEADER_ONLY=1 -DURDL_DISABLE_SSL=1 -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-deprecated-declarations
 BOOSTLIBS = -lboost_filesystem -lboost_thread -lboost_system -lboost_regex -lboost_log -lboost_timer -lboost_chrono -lboost_program_options -lboost_iostreams -lboost_serialization ${ROOT}/third_party/bzip2/lib/libbz2.a -lrt -L${JEMALLOC}/lib -Wl,-rpath,${JEMALLOC}/lib -ljemalloc
 THIRDLIBS = ../third_party/libexpat_linux.a
endif


# Servers
ifeq ($(host),new_server)
 CXX = g++ -DGIT_VERSION=\"$(GIT_VERSION)\" -DGIT_URL=\"$(GIT_URL)\"
 LD  = g++
 LDFLAGS   = -pthread
 MONGOLIBS = /local/data/libraries/mongo-cxx-driver/lib/libmongoclient.a
 BOOST_DIR = /local/data/libraries/boost/lib
 DEFCXXFLAGS = -std=c++11 -Wall -O3 -ffast-math -march=native -Ofast -I/local/data/libraries/mongo-cxx-driver/include/ -I/local/data/libraries/boost/include/ -I$(MINILZO) -I$(URDL) -I$(CPPFORMAT) -I$(STRTK) -I$(LUA) -DURDL_HEADER_ONLY=1 -DURDL_DISABLE_SSL=1 -I${JEMALLOC}/include
 #DEFCXXFLAGS = -std=c++11 -I/local/data/libraries/mongo-cxx-driver/include/ -I/local/data/libraries/boost/include/ -I$(MINILZO) -I$(URDL) -I$(CPPFORMAT) -I$(STRTK) -I$(LUA) -DURDL_HEADER_ONLY=1 -DURDL_DISABLE_SSL=1 -I${JEMALLOC}/include
 BOOSTLIBS = $(BOOST_DIR)/libboost_filesystem.a $(BOOST_DIR)/libboost_thread.a $(BOOST_DIR)/libboost_system.a $(BOOST_DIR)/libboost_regex.a $(BOOST_DIR)/libboost_log.a  $(BOOST_DIR)/libboost_timer.a  $(BOOST_DIR)/libboost_chrono.a $(BOOST_DIR)/libboost_program_options.a $(BOOST_DIR)/libboost_iostreams.a $(BOOST_DIR)/libboost_serialization.a /local/data/DeepBlue/server/third_party/bzip2/lib/libbz2.a -lrt -L${JEMALLOC}/lib -Wl,-rpath,${JEMALLOC}/lib -ljemalloc
 THIRDLIBS = ../third_party/libexpat_linux.a
endif

ifeq ($(host),production)
 CXX = g++ -DGIT_VERSION=\"$(GIT_VERSION)\" -DGIT_URL=\"$(GIT_URL)\"
 LD  = g++
 LDFLAGS   = -pthread
 MONGOLIBS = /opt/mongodb/install/mongo-driver/lib/libmongoclient.a
 BOOST_DIR = /opt/mongodb/boost/lib
 DEFCXXFLAGS = -std=c++11 -Wall -O3 -ffast-math -march=native -Ofast -I/opt/mongodb/install/mongo-driver/include/ -I/opt/mongodb/boost/include/ -I$(MINILZO) -I$(URDL) -I$(CPPFORMAT) -I$(STRTK) -I$(LUA) -DURDL_HEADER_ONLY=1 -DURDL_DISABLE_SSL=1 -I${JEMALLOC}/include -I${JEMALLOC}/include
 BOOSTLIBS = $(BOOST_DIR)/libboost_filesystem.a $(BOOST_DIR)/libboost_thread.a $(BOOST_DIR)/libboost_system.a $(BOOST_DIR)/libboost_regex.a $(BOOST_DIR)/libboost_log.a  $(BOOST_DIR)/libboost_timer.a  $(BOOST_DIR)/libboost_chrono.a $(BOOST_DIR)/libboost_program_options.a $(BOOST_DIR)/libboost_iostreams.a /local/data/DeepBlue/server/third_party/bzip2/lib/libbz2.a -lrt -L${JEMALLOC}/lib -Wl,-rpath,${JEMALLOC}/lib -ljemalloc
 THIRDLIBS = ../third_party/libexpat_linux.a
endif

# -O3 -Ofast
# Felipe Albrecht's mac
ifeq ($(host),macosx)
 CXX = clang++ -DGIT_VERSION=\"$(GIT_VERSION)\" -DGIT_URL=\"$(GIT_URL)\"
 NO_WARNINGS = -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-deprecated-declarations -Wno-unused-local-typedefs -Wno-redeclared-class-member
 # -g -O3 -Ofast -ffast-math -march=native
 # The no warnings: -Wno-redeclared-class-member are declared because the boost set_of.hpp library
 DEFCXXFLAGS = -O3 -Ofast -ffast-math -march=native -std=c++11 -stdlib=libc++ -Werror -Wall -Wextra -Weffc++ $(NO_WARNINGS) -I/opt/boost-new-new/include/ -I/Users/albrecht/mpi/DeepBlue/server/third_party/drivers/macosx/include -I$(CPPFORMAT) -I$(STRTK) -I$(MINILZO) -I$(URDL) -I$(LUA) -DBOOST_LOG_DYN_LINK -DURDL_HEADER_ONLY=1 -DURDL_DISABLE_SSL=1
 BOOST_DIR = /opt/boost-new-new/lib
 LD	= clang++ -DBOOST_LOG_DYN_LINK -pagezero_size 10000 -image_base 100000000  #LuaJit options
 LDFLAGS = -pthread
 MONGOLIBS =  -L/Users/albrecht/mpi/DeepBlue/server/third_party/drivers/macosx/lib -lmongoclient
 BOOSTLIBS = -L$(BOOST_DIR) -lboost_log_setup -lboost_log -lboost_filesystem -lboost_thread -lboost_system -lboost_regex -lboost_timer -lboost_chrono -lboost_program_options -lboost_iostreams
 THIRDLIBS = ../third_party/libexpat_macosx.a
endif

CPPFORMAT_LIB = ../third_party/cppformat/format.cc ../third_party/minilzo/minilzo.c

DEFLIBS = $(CPPFORMAT_LIB) $(MONGOLIBS) $(BOOSTLIBS) -L$(LUA) -lluajit

AR	= ar
ARFLAGS	=
RANLIB	= ranlib

RM	= rm
ECHO	= echo

SHELL	= /bin/sh
