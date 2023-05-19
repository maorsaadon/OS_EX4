#include "Reactor.h"

// The reactor pointer.
void *thisReactor = NULL;
int client_count = 0;
int total_bytes = 0;

int main(void)
{

	signal(SIGINT, signalHandler);

	// Create socket
	int serverFd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (serverFd == -1)
	{
		perror("Could not create socket\n");
		exit(-1);
	}

	int enableReuse = 1;
	if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0)
	{
		perror("setsockopt() failed\n");
		exit(-1);
	}

	struct sockaddr_in serverAddress = {0};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	// Bind socket to address and port
	if (bind(serverFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
	{
		perror("Bind() failed");
		close(serverFd);
		exit(-1);
	}

	int listenResult = listen(serverFd, CLIENTS);
	if (listenResult == -1)
	{
		perror("listen() failed\n");
		close(serverFd);
		exit(-1);
	}

	fprintf(stdout, "Server listening on port %d\n", PORT);

	thisReactor = createReactor();

	if (thisReactor == NULL)
	{
		perror("createReactor() failed");
		exit(-1);
	}

	fprintf(stdout, "Adding server socket to reactor...\n");
	addFd(thisReactor, serverFd, serverHandler);
	fprintf(stdout, "Server socket added to reactor.\n");

	startReactor(thisReactor);
	WaitFor(thisReactor);
	signalHandler();

	return 0;
}

void signalHandler()
{
	fprintf(stdout, "Server shutting down...\n");

	if (thisReactor != NULL)
	{
		stopReactor(thisReactor);

		fprintf(stdout, "Closing all sockets and freeing memory...\n");

		preactor reactor = (preactor)thisReactor;

		// free the hashmap
		hashmap_iterate(reactor->FDtoFunction, free_entry, NULL);
		hashmap_free(reactor->FDtoFunction);

		// free pfds
		free(reactor->pfds);

		// free reactor
		free(reactor);
	}

	else
		fprintf(stdout, "Reactor wasn't created, no memory cleanup needed.\n");

	exit(1);
}

void *clientHandler(int clientFd, void *arg)
{
	char buffer[BUFFER_SIZE] = {0};

	ssize_t nbytes = recv(clientFd, buffer, sizeof(buffer), 0);
	if (nbytes <= 0)
	{
		perror("recv() failed\n");
	}

	total_bytes += nbytes;

	buffer[nbytes] = '\0';

	fprintf(stdout, "Client %d: %s\n", clientFd, buffer);

	return arg;
}

void *serverHandler(int serverFd, void *arg)
{
	struct sockaddr_in clientAddress = {0};
	socklen_t len_clientAddress = sizeof(clientAddress);

	preactor reactor = (preactor)arg;

	if (reactor == NULL)
	{
		perror("serverHandler() failed");
		exit(-1);
	}

	int clientFd = accept(serverFd, (struct sockaddr *)&clientAddress, &len_clientAddress);
	if (clientFd == -1)
	{
		perror("listen() failed\n");
		close(serverFd);
		exit(-1);
	}
	else
		printf("A new client connection accepted\n");

	addFd(reactor, clientFd, clientHandler);

	client_count++;

	fprintf(stdout, "Client %s : %d connected, ID: %d.\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), clientFd);

	return arg;
}