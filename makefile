CC=gcc
FLAGS=-Wall -g -pthread -o

all:reactorlib.so react_server 

reactorlib.so: Reactor.c 
	$(CXX) -shared -fPIC -o Reactor.cpp cpplib.so 

react_server: reactorlib.so Server.c Server.h map.h
	$(CXX) $(FLAGS) react_server Server.c map.c ./reactorlib.so


clean:
	rm -f *.o *.so react_server
	


