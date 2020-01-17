# cse3310 project sample makefile



CXX=g++
# note the below asio package may be in a 
# different place
CPPFLAGS=-I../asio-1.13.0/include
CXXFLAGS=-Wall -g -std=c++11 -O0

# many examples on the web show the 'pkg-config' commands
# being executed during the make process.  this just
# does it once when the makefile is initially parsed
GTKLINKFLAGS=$(shell pkg-config --libs gtk+-3.0)
GTKCOMPILEFLAGS=$(shell pkg-config --cflags gtk+-3.0)

TARGETS=chat_server gtk_client

all:${TARGETS}


# these compile and link in the same command.
# no intermediate object code files
# (sacrifice compilation speed for not having to
#  deal with dependencies)

gtk_client:  gtk_client.cpp
	${CXX} ${CXXFLAGS} ${GTKCOMPILEFLAGS} ${CPPFLAGS} -o $@ $< \
           ${GTKLINKFLAGS} -lpthread 

chat_server: chat_server.cpp chat_message.hpp table.hpp game.hpp
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -o $@ $< -lpthread


clean:
	-rm -f ${TARGETS}
