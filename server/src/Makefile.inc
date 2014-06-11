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
 DEFCXXFLAGS = -Wall -O3 -Ofast -I/opt/mongodb/boost/include/ -I/opt/mongodb/driver/include/
 BOOSTLIBS = $(BOOST_DIR)/libboost_filesystem.a $(BOOST_DIR)/libboost_thread.a $(BOOST_DIR)/libboost_system.a $(BOOST_DIR)/libboost_regex.a $(BOOST_DIR)/libboost_log.a  $(BOOST_DIR)/libboost_timer.a  $(BOOST_DIR)/libboost_chrono.a -lrt
endif

# Felipe Albrecht's mac
ifeq ($(host),macosx)
 CXX = clang++
 DEFCXXFLAGS = -Werror -Wall -Wextra -Weffc++ -Wno-unused-parameter -Wno-ignored-qualifiers -DPROFINY_CALL_GRAPH_PROFILER -g -I/opt/boost/include/ -I/Users/albrecht/mpi/epidb/epidb/mongo/driver/macosx/include
 BOOST_DIR = /opt/boost/lib
 LD	= clang++
 LDFLAGS = -g
 MONGOLIBS =  -L/Users/albrecht/mpi/epidb/epidb/mongo/driver/macosx/lib -lmongoclient
 BOOSTLIBS = $(BOOST_DIR)/libboost_filesystem.a $(BOOST_DIR)/libboost_thread.a $(BOOST_DIR)/libboost_system.a $(BOOST_DIR)/libboost_regex.a $(BOOST_DIR)/libboost_log.a  $(BOOST_DIR)/libboost_timer.a  $(BOOST_DIR)/libboost_chrono.a
endif

DEFLIBS = $(MONGOLIBS) $(BOOSTLIBS)
THIRDLIBS = ../third_party/libexpat_macosx.a

AR	= ar
ARFLAGS	=
RANLIB	= ranlib

RM	= rm
ECHO	= echo

SHELL	= /bin/sh
