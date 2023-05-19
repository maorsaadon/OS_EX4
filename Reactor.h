#ifndef REACTOR_H
#define REACTOR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#include "map.h"


#define PORT 9034
#define CLIENTS 8192
#define BUFFER_SIZE 1024

typedef void *(*handler_t)(int fd, void *arg);


typedef struct reactor
{
	struct pollfd *pfds;
    int clients_counter;
    int fd_size;
    struct hashmap *FDtoFunction;
    bool hot;
    pthread_t *thread;
} reactor, *preactor;

void *createReactor();
void startReactor(void *this);
void stopReactor(void *this);
void addFd(void *this, int fd, handler_t handler);
void WaitFor(void *this);

void signalHandler();			
void *clientHandler(int fd, void *arg);
void *serverHandler(int fd, void *arg);


void free_entry(void *key, size_t ksize, uintptr_t value, void *usr)
{
    free((int *)key);
}

#endif