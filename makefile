CC=gcc
FLAGS=-Wall -g -pthread -o

all: libreactor.so reactor_server

libreactor.so: Reactor.c
	$(CC) -shared -fPIC -o libreactor.so Reactor.c

reactor_server: libreactor.so Server.c Reactor.h map.h
	$(CC) $(FLAGS) reactor_server Server.c map.c ./libreactor.so
clean:
	rm reactor_server *.so 