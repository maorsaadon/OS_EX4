CC=gcc
FLAGS=-Wall -g -pthread -o

all: libreactor.so react_server

libreactor.so: Reactor.c
	$(CC) -shared -fPIC -o libreactor.so Reactor.c

react_server: libreactor.so Server.c Reactor.h map.h
	$(CC) $(FLAGS) react_server Server.c map.c ./libreactor.so
clean:
	rm react_server *.so 