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

typedef void *(*handler_t)(int fd, void *react);

// Structure to represent the reactor
typedef struct reactor
{
    struct pollfd *pfds;          // Array of poll file descriptors
    int clients_counter;          // Number of active clients
    int size;                     // Size of the pfds array
    struct hashmap *FDtoFunction; // Hashmap to store file descriptors and their handlers
    bool hot;                     // Flag to indicate if the reactor is active
    pthread_t thread;             // Thread ID of the reactor thread
} reactor, *preactor;

// Function to create a new reactor
void *createReactor();

// Function to start the reactor
void startReactor(void *thisReactor);

// Function to stop the reactor
void stopReactor(void *thisReactor);

// Function to add a file descriptor and its corresponding handler to the reactor
void addFd(void *thisReactor, int fd, handler_t handler);

// Function to wait for the reactor thread to finish
void WaitFor(void *thisReactor);

// Signal handler function for handling termination signals
void signalHandler();

// Client handler function for processing client connections
void *clientHandler(int fd, void *arg);

// Server handler function for processing server connections
void *serverHandler(int fd, void *arg);

// Function to free hashmap entries
void free_entry(void *key, size_t ksize, uintptr_t value, void *usr)
{
    free((int *)key);
}

#endif
