#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h> // Include netdb.h for struct addrinfo
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Reactor.h"

#define PORT "9034" // the port client will be connecting to
#define MAXDATASIZE 1024 // max number of bytes we can get at once

int clientfd;

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int get_client_fd(int argc, char *argv[]){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        exit(2);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr),
              s, sizeof s);
    printf("client: connected to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    return sockfd;
}

void recv_handler(int *fd){
    char buf[1024];
    int bytes;
    if ((bytes = recv(*fd, buf, MAXDATASIZE - 1, 0)) <= 0) {
        perror("recv");
        exit(1);
    }
    buf[bytes] = '\0';
    printf("%s", buf);
}

void send_handler(int *fd){
    size_t line_len = 0;
    char *message = NULL;
    getline(&message, &line_len, stdin);
    if (send(clientfd, message, strlen(message), 0) == -1) {
        perror("socket send at send command");
        exit(1);
    }
    free(message);
}

int main(int argc, char *argv[]) {
    pReactor pr = (pReactor) newReactor();
    clientfd = get_client_fd(argc, argv);
    InstallHandler(pr, (void (*)(void*)) recv_handler, clientfd);
    InstallHandler(pr, (void (*)(void*)) send_handler, STDIN_FILENO);
    pthread_join(pr->thread, NULL);

    return 0;
}
