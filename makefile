CC = gcc
CXX = g++
FLAGS = -Wall -g -w

all:react_server

react_server: Server.c Reactor.h map.h clib.so 
	$(CXX) $(FLAGS) Server.c map.c ./clib.so -o react_server

# shared library for all the c++ code files
clib.so: Reactor.c
	$(CXX) $(FLAGS) --shared -fPIC Reactor.c -o clib.so -lpthread


clean:
	rm -f *.o *.so react_server