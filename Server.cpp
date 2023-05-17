#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include "Reactor.hpp"

#define PORT "9034" // Port we're listening on

pReactor pr; /// global reactor to be use to manage the communication of the server

void when_exit(int signal)
{
    delReactor(pr);
    exit(0);
}

/**
 * Get sockaddr, IPv4 or IPv6:
 * @param sa
 * @return return the ip of the sockaddr struct
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

/**
 * This function initial a new listener fd
 * @return listening socket
 */
int get_listener_socket()
{
    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints = {0}, *ai = {0}, *p = {0};

    // Get us a socket and bind it
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL)
    {
        return -1;
    }

    // Listen
    if (listen(listener, 10) == -1)
    {
        return -1;
    }

    return listener;
}

/**
 * This function attached to the reactor to handle the communication from the client
 * @param newfd - the client fd
 */
void handle_client(int *newfd)
{
    char buff[1024];
    memset(buff, '\0', 1024);
    int bytes = read(*newfd, buff, 1024);
    if (bytes < 0)
    {
        perror("read from fd error");
    }
    else if (bytes == 0)
    {
        printf("pollserver: client seems to be off, remove his handler... \n");
        RemoveHandler((void *)pr, *newfd);
        close(*newfd);
    }
    else
    {
        for (int i = 0; i < pr->handlers_size; ++i)
        {
            if ((void (*)(int *))pr->funcs[i] == handle_client && pr->pfds[i].fd != *newfd)
            { // if the fd is clientfd then broadcast
                send(pr->pfds[i].fd, buff, bytes, 0);
            }
        }
    }
}

/**
 * This function is handler function for the listener fd
 * @param listener - the listener fd
 */
void accept_clients(int *listener)
{
    int newfd;                          // Newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // Client address
    char remoteIP[INET6_ADDRSTRLEN];
    socklen_t addrlen = sizeof(remoteaddr);

    newfd = accept(*listener, (struct sockaddr *)&remoteaddr, &addrlen);

    if (newfd == -1)
    {
        perror("accept");
    }
    else
    {
        InstallHandler(pr, (void (*)(void *))handle_client, newfd);

        printf("pollserver: new connection from %s on socket %d\n",
               inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP,
                         INET6_ADDRSTRLEN),
               newfd);
    }
}

int main()
{

    signal(SIGINT, when_exit);

    pr = (pReactor)newReactor();

    // Set up and get a listening socket
    int listener = get_listener_socket();

    if (listener == -1)
    {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    InstallHandler(pr, (void (*)(void *))accept_clients, listener);
    printf("pollserver: waiting for new connections...\n");
    pthread_join(pr->thread, NULL);
    close(listener);

    return 0;
}