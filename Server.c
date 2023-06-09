#include "Reactor.h"
#include "map.h"

void *thisReactor = NULL;
int client_count = 0;
int total_bytes = 0;

// Signal handler for handling termination signals
void signalHandler();

// Main function
int main(void)
{
	signal(SIGINT, signalHandler);

	// Create socket
	int serverFd = socket(AF_INET, SOCK_STREAM, 0);
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

	// Set up server address
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

	// Start listening for incoming connections
	int listenResult = listen(serverFd, CLIENTS);
	if (listenResult == -1)
	{
		perror("listen() failed\n");
		close(serverFd);
		exit(-1);
	}

	fprintf(stdout, "Server listening on port %d\n", PORT);

	// Create the reactor
	thisReactor = createReactor();

	if (thisReactor == NULL)
	{
		perror("createReactor() failed");
		exit(-1);
	}

	fprintf(stdout, "Adding server socket to reactor...\n");
	// Add server socket to the reactor
	addFd(thisReactor, serverFd, serverHandler);
	fprintf(stdout, "Server socket added to reactor.\n");

	// Start the reactor
	startReactor(thisReactor);
	WaitFor(thisReactor);
	signalHandler();

	return 0;
}

// Signal handler function for handling termination signals
void signalHandler()
{
	fprintf(stdout, "Server shutting down...\n");

	if (thisReactor != NULL)
	{
		// Stop the reactor
		stopReactor(thisReactor);

		fprintf(stdout, "Closing all sockets and freeing memory...\n");

		preactor reactor = (preactor)thisReactor;
		// Free the hashmap
		hashmap_iterate(reactor->FDtoFunction, free_entry, NULL);
		hashmap_free(reactor->FDtoFunction);
		// Free pfds
		free(reactor->pfds);
		// Free reactor
		free(reactor);
	}
	else
	{
		fprintf(stdout, "Reactor wasn't created, no memory cleanup needed.\n");
	}

	exit(1);
}

// Client handler function for processing client connections
void *clientHandler(int clientFd, void *react)
{
	char buffer[BUFFER_SIZE] = {0};

	// Receive data from the client
	ssize_t nbytes = recv(clientFd, buffer, sizeof(buffer), 0);
	if (nbytes <= 0)
	{
		if (nbytes == -1)
			perror("recv() failed");

		else
			printf("Client disconnected\n");

		close(clientFd);

		preactor reactor = (preactor)react;
		// Connection closed or error occurred
		int disconnectedFd = clientFd;
		int i = 0;
		// Remove the disconnected fd from the list
		for (size_t j = 0; j < reactor->clients_counter; j++)
		{
			if (reactor->pfds[j].fd == disconnectedFd)
			{
				i = j;
				break;
			}
		}

		if (i < reactor->clients_counter - 1)
		{
			for (size_t j = i; j < reactor->clients_counter - 1; j++)
			{
				reactor->pfds[j] = reactor->pfds[j + 1];
			}
			reactor->clients_counter--;
		}

		else
		{
			reactor->pfds[i].fd = -1;
		}

		reactor->clients_counter--;

		// Remove the fd from the hashmap
		hashmap_remove(reactor->FDtoFunction, &disconnectedFd, sizeof(int));
		return NULL;
	}

	total_bytes += nbytes;

	// Make sure the buffer is null-terminated, so we can print it.
	if (nbytes < BUFFER_SIZE)
		buffer[nbytes] = '\0';
	else
		buffer[BUFFER_SIZE - 1] = '\0';

	// Remove the arrow keys from the buffer, as they are not printable and mess up the output.
	// And replace them with spaces, so the rest of the message won't cut off.
	for (int i = 0; i < nbytes - 3; i++)
	{
		if ((buffer[i] == 0x1b) && (buffer[i + 1] == 0x5b) && (buffer[i + 2] == 0x41 || buffer[i + 2] == 0x42 || buffer[i + 2] == 0x43 || buffer[i + 2] == 0x44))
		{
			buffer[i] = 0x20;
			buffer[i + 1] = 0x20;
			buffer[i + 2] = 0x20;

			i += 2;
		}
	}

	fprintf(stdout, "Client %d: %s\n", clientFd, buffer);

	return react;
}

// Server handler function for processing server connections
void *serverHandler(int serverFd, void *react)
{
	struct sockaddr_in clientAddress = {0};
	socklen_t len_clientAddress = sizeof(clientAddress);

	preactor reactor = (preactor)react;

	if (reactor == NULL)
	{
		perror("serverHandler() failed");
		exit(-1);
	}

	// Accept incoming client connections
	int clientFd = accept(serverFd, (struct sockaddr *)&clientAddress, &len_clientAddress);
	if (clientFd == -1)
	{
		perror("listen() failed\n");
		close(serverFd);
		exit(-1);
	}
	else
	{
		printf("A new client connection accepted\n");
	}

	// Add client socket to the reactor
	addFd(reactor, clientFd, clientHandler);

	client_count++;

	fprintf(stdout, "Client %s : %d connected, ID: %d.\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), clientFd);

	return react;
}
