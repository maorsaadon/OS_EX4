CC = gcc
CXX = g++
FLAGS = -Wall -g -w

all: react_server 

react_server: Server.cpp cpplib.so
	$(CXX) $(FLAGS) Server.cpp ./clib.so -o Server

cpplib.so: Reactor.cpp Reactor.hpp 
	$(CXX) $(FLAGS) --shared -fPIC Reactor.cpp  Reactor.hpp -o clib.so -lpthread

.PHONY : clean
clean:
	rm -f *.o *.so react_server