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
	ifeq ($(UNAME_N),infao3901)
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
 LDFLAGS   = -pthread
 MONGOLIBS = -L/local/data/DeepBlue/server/third_party/drivers/linux/lib -lmongoclient
 BOOST_DIR = /opt/mongodb/boost/lib
 DEFCXXFLAGS = -Wall -O3 -march=native -Ofast -I/opt/mongodb/boost/include/ -I/local/data/DeepBlue/server/third_party/drivers/linux/include/ -I/local/data/DeepBlue/server/third_party/minilzo/ -I/local/data/DeepBlue/server/third_party/cppformat/
 BOOSTLIBS = $(BOOST_DIR)/libboost_filesystem.a $(BOOST_DIR)/libboost_thread.a $(BOOST_DIR)/libboost_system.a $(BOOST_DIR)/libboost_regex.a $(BOOST_DIR)/libboost_log.a  $(BOOST_DIR)/libboost_timer.a  $(BOOST_DIR)/libboost_chrono.a $(BOOST_DIR)/libboost_program_options.a -lrt
 THIRDLIBS = ../third_party/libexpat_linux.a

endif

# -O3 -Ofast
# Felipe Albrecht's mac
ifeq ($(host),macosx)
 CXX = clang++
 # -O3 -Ofast -ffast-math -march=native
 DEFCXXFLAGS = -O3 -Ofast -ffast-math -march=native -std=c++11 -stdlib=libc++ -Werror -Wall -Wextra  -Weffc++ -Wno-unused-parameter -Wno-ignored-qualifiers -I/opt/boost-new-new/include/ -I/Users/albrecht/mpi/DeepBlue/server/third_party/drivers/macosx_new/include -I/Users/albrecht/mpi/DeepBlue/server/third_party/cppformat/ -I/Users/albrecht/mpi/DeepBlue/server/third_party/strtk -I/Users/albrecht/mpi/DeepBlue/server/third_party/minilzo/
 BOOST_DIR = /opt/boost-new-new/lib
 LD	= clang++
 LDFLAGS =
 MONGOLIBS =  -L/Users/albrecht/mpi/DeepBlue/server/third_party/drivers/macosx_new/lib -lmongoclient
 BOOSTLIBS = -L$(BOOST_DIR) -lboost_log_setup -lboost_log -lboost_filesystem -lboost_thread -lboost_system -lboost_regex -lboost_timer -lboost_chrono -lboost_program_options
 THIRDLIBS = ../third_party/libexpat_macosx.a
endif

CPPFORMAT = ../third_party/cppformat/format.cc ../third_party/minilzo/minilzo.c
DEFLIBS = $(CPPFORMAT) $(BOOSTLIBS) $(MONGOLIBS)

AR	= ar
ARFLAGS	=
RANLIB	= ranlib

RM	= rm
ECHO	= echo

SHELL	= /bin/sh
