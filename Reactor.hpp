#ifndef DESIGN_PATTERNS_REACTOR_HPP
#define DESIGN_PATTERNS_REACTOR_HPP

#include <poll.h>
#include <pthread.h>
#include <unistd.h>

#define REACTOR_SIZE sizeof(struct reactor)

typedef struct reactor
{
    struct pollfd *pfds;    // list of fds to listen on
    void (**funcs)(void *); // list of handlers function
    pthread_t thread;       // the thread who run the handlers
    int handlers_size;
    /// Every fd in the pfds[i] get handle by a function from funcs[i]
} *pReactor;

void *newReactor();
void InstallHandler(void *reactor, void (*func)(void *), int fd);
void RemoveHandler(void *reactor, int fd);
void *activate_poll_listener(void *reactor);
void delReactor(pReactor pr); /// delete and free al the reactor

#endif // DESIGN_PATTERNS_REACTOR_HPP