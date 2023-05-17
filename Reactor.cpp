#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "Reactor.hpp"

void *activate_poll_listener(void *reactor)
{
    pReactor pr = (pReactor)reactor;
    while (1)
    {
        poll(pr->pfds, pr->handlers_size, -1); // -1 mean making the poll wait forever (with no timeout)
        for (int i = 0; i < pr->handlers_size; ++i)
        { // for all the fds
            if (pr->pfds[i].revents & POLLIN)
            {                                    // if the events is one we read from the fd
                pr->funcs[i](&(pr->pfds[i].fd)); // run the suitable function[i] on fd[i]
            }
        }
    }
}

void *newReactor()
{
    pReactor reactor = (pReactor)malloc(REACTOR_SIZE);
    if (reactor == NULL)
    {
        perror("reactor malloc error");
        exit(1);
    }
    reactor->handlers_size = 0;
    reactor->pfds = NULL;
    reactor->funcs = NULL;
    return reactor;
}

void InstallHandler(void *reactor, void (*func)(void *), int fd)
{
    pReactor pr = (pReactor)reactor;

    if (pr->handlers_size == 0)
    {
        pr->pfds = (struct pollfd *)malloc(sizeof(struct pollfd));
        if (pr->pfds == NULL)
        {
            perror("reactor malloc error");
            exit(1);
        }
        pr->pfds[0].fd = fd;         // adding the fd to the fd list
        pr->pfds[0].events = POLLIN; // set the "event listener" to input

        pr->funcs = (void (**)(void *))malloc(8);
        if (pr->funcs == NULL)
        {
            perror("reactor malloc error");
            exit(1);
        }
        pr->funcs[0] = func; // adding the function to the handler list
        pr->handlers_size += 1;
        /// since its the first handler installation we need to create the handle thread
        pthread_create(&pr->thread, NULL, activate_poll_listener, reactor);
    }
    else
    {

        pr->handlers_size += 1;
        pr->pfds = (struct pollfd *)realloc(pr->pfds, sizeof(struct pollfd) * pr->handlers_size);
        if (pr->pfds == NULL)
        {
            perror("reactor malloc error");
            exit(1);
        }
        pr->pfds[pr->handlers_size - 1].fd = fd;         // adding the fd to the fd list
        pr->pfds[pr->handlers_size - 1].events = POLLIN; // set the "event listener" to input

        pr->funcs = (void (**)(void *))realloc(pr->funcs, 8 * pr->handlers_size);
        if (pr->funcs == NULL)
        {
            perror("reactor malloc error");
            exit(1);
        }
        pr->funcs[pr->handlers_size - 1] = func; // adding the function to the handler list
    }
}

void RemoveHandler(void *reactor, int fd)
{
    pReactor pr = (pReactor)reactor;
    int fd_index = -1;
    for (int i = 0; i < pr->handlers_size; ++i)
    {
        if (pr->pfds[i].fd == fd)
        {
            fd_index = i;
            break;
        }
    }
    if (fd_index == -1)
    { // in case the fd doesn't exist
        return;
    }
    if (pr->handlers_size == 1)
    { // in case this is the last
        /// since we remove the last handler we need to cancel the handler thread
        pthread_cancel(pr->thread);
        free(pr->funcs);
        pr->funcs = NULL;
        free(pr->pfds);
        pr->pfds = NULL;
        pr->handlers_size = 0;
    }
    else
    {
        --(pr->handlers_size);
        struct pollfd *newpfds = (struct pollfd *)malloc(sizeof(struct pollfd) * pr->handlers_size);
        void (**newfuncs)(void *) = (void (**)(void *))malloc(8 * pr->handlers_size);

        if (fd_index != 0)
        {
            memcpy(newpfds, pr->pfds, fd_index * sizeof(struct pollfd)); // copy everything BEFORE the index
            memcpy(newfuncs, pr->funcs, fd_index * 8);                   // copy everything BEFORE the index
        }

        if (fd_index != pr->handlers_size)
        {
            memcpy(newpfds + fd_index, pr->pfds + fd_index + 1,
                   (pr->handlers_size - fd_index) * sizeof(struct pollfd)); // copy everything AFTER the index
            memcpy(newfuncs + fd_index, pr->funcs + fd_index + 1,
                   (pr->handlers_size - fd_index) * sizeof(struct pollfd)); // copy everything AFTER the index
        }
        free(pr->funcs);
        free(pr->pfds);
        pr->pfds = newpfds;
        pr->funcs = newfuncs;
    }
}

void delReactor(pReactor pr)
{
    if (pr->handlers_size != 0)
    {
        free(pr->pfds);
        free(pr->funcs);
    }
    free(pr);
}