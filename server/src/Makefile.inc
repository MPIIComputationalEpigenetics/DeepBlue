UNAME_S := $(shell uname -s)
UNAME_N := $(shell uname -n)


ifeq ($(UNAME_S),Linux)
	host = linux
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
 

#Root Directory
ifeq ($(host),new_server)
	ROOT=/local/data/DeepBlue/server/
endif
ifeq ($(host),production)
	ROOT=/local/data/DeepBlue/server/
endif
ifeq ($(host),macosx)
	ROOT=/Users/albrecht/mpi/DeepBlue/server/
endif
ifeq ($(host),ubuntu)
	ROOT=/home/albrecht/projects/DeepBlue/server
endif

LUA=$(ROOT)/third_party/luajit-2.0/sr
MINILZO=$(ROOT)/third_party/minilzo
STRTK=$(ROOT)/third_party/strtk
CPPFORMAT=$(ROOT)/third_party/cppformat
URDL=$(ROOT)/third_party/urdl-0.1/include

ifeq ($(host),ubuntu)
 CXX = g++
 LD  = g++ -DBOOST_LOG_DYN_LINK
 LDFLAGS   = -pthread 
 MONGOLIBS = /opt/mongo/lib/libmongoclient.a
 BOOST_DIR = /opt/boost/lib
 DEFCXXFLAGS = -std=c++11 -Wall -O3 -ffast-math -march=native -Ofast -I/opt/mongo/include/ -I/opt/boost/include/ -I$(MINILZO) -I$(URDL) -I$(CPPFORMAT) -I$(STRTK) -I$(LUA) -DURDL_HEADER_ONLY=1 -DURDL_DISABLE_SSL=1
 BOOSTLIBS = $(BOOST_DIR)/libboost_filesystem.a $(BOOST_DIR)/libboost_thread.a $(BOOST_DIR)/libboost_system.a $(BOOST_DIR)/libboost_regex.a $(BOOST_DIR)/libboost_log.a  $(BOOST_DIR)/libboost_timer.a  $(BOOST_DIR)/libboost_chrono.a $(BOOST_DIR)/libboost_program_options.a $(BOOST_DIR)/libboost_iostreams.a /home/albrecht/projects/DeepBlue/server/third_party/bzip/lib/libbz2.a -lrt
 THIRDLIBS = ../third_party/libexpat_linux.a
endif


# Servers
ifeq ($(host),new_server)
 CXX = g++
 LD  = g++
 LDFLAGS   = -pthread
 MONGOLIBS = /local/data/libraries/mongo-cxx-driver/lib/libmongoclient.a
 BOOST_DIR = /local/data/libraries/boost/lib
 DEFCXXFLAGS = -std=c++11 -Wall -O3 -ffast-math -march=native -Ofast -I/local/data/libraries/mongo-cxx-driver/include/ -I/local/data/libraries/boost/include/ -I$(MINILZO) -I$(URDL) -I$(CPPFORMAT) -I$(STRTK) -I$(LUA) -DURDL_HEADER_ONLY=1 -DURDL_DISABLE_SSL=1
 BOOSTLIBS = -lbzip2 $(BOOST_DIR)/libboost_filesystem.a $(BOOST_DIR)/libboost_thread.a $(BOOST_DIR)/libboost_system.a $(BOOST_DIR)/libboost_regex.a $(BOOST_DIR)/libboost_log.a  $(BOOST_DIR)/libboost_timer.a  $(BOOST_DIR)/libboost_chrono.a $(BOOST_DIR)/libboost_program_options.a -lboost_iostreams -lrt
 THIRDLIBS = ../third_party/libexpat_linux.a
endif

ifeq ($(host),production)
 CXX = g++
 LD  = g++
 LDFLAGS   = -pthread
 MONGOLIBS = /opt/mongodb/install/mongo-driver/lib/libmongoclient.a
 BOOST_DIR = /opt/mongodb/boost/lib
 DEFCXXFLAGS = -std=c++11 -Wall -O3 -ffast-math -march=native -Ofast -I/opt/mongodb/install/mongo-driver/include/ -I$(MINILZO) -I$(URDL) -I$(CPPFORMAT) -I$(STRTK) -I$(LUA) -DURDL_HEADER_ONLY=1 -DURDL_DISABLE_SSL=1
 BOOSTLIBS = $(BOOST_DIR)/libboost_filesystem.a $(BOOST_DIR)/libboost_thread.a $(BOOST_DIR)/libboost_system.a $(BOOST_DIR)/libboost_regex.a $(BOOST_DIR)/libboost_log.a  $(BOOST_DIR)/libboost_timer.a  $(BOOST_DIR)/libboost_chrono.a $(BOOST_DIR)/libboost_program_options.a $(BOOST_DIR)/libboost_iostreams.a /local/data/DeepBlue/server/third_party/bzip2-1.0.6/lib/lib/libbz2.a -lrt
 THIRDLIBS = ../third_party/libexpat_linux.a
endif

# -O3 -Ofast
# Felipe Albrecht's mac
ifeq ($(host),macosx)
 CXX = clang++
 # -g -O3 -Ofast -ffast-math -march=native
 DEFCXXFLAGS = -O3 -Ofast -ffast-math -march=native -std=c++11 -stdlib=libc++ -Werror -Wall -Wextra  -Weffc++ -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-deprecated-declarations -I/opt/boost-new-new/include/ -I/Users/albrecht/mpi/DeepBlue/server/third_party/drivers/macosx/include -I$(CPPFORMAT) -I$(STRTK) -I$(MINILZO) -I$(URDL) -I$(LUA) -DBOOST_LOG_DYN_LINK -DURDL_HEADER_ONLY=1 -DURDL_DISABLE_SSL=1
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
